/*
 * Copyright (C) 2016 Peter Gazdik
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "MarkovPassGen.h"

#include <arpa/inet.h>		// ntohl, ntohs
#include <cstdlib>				// atoi, qsort

#include <limits>
#include <stdexcept>
#include <iostream>
#include <sstream>

using namespace std;

uint8_t * MarkovPassGen::_markov_table[MAX_PASS_LENGTH][CHARSET_SIZE];
uint64_t MarkovPassGen::_length_permut[MAX_PASS_LENGTH + 1];
unsigned MarkovPassGen::_min_length;
unsigned MarkovPassGen::_max_length;
unsigned MarkovPassGen::_step;
std::mutex MarkovPassGen::_mutex;
unsigned MarkovPassGen::_thresholds[MAX_PASS_LENGTH];

MarkovPassGen::MarkovPassGen(MarkovPassGenOptions & options)
{
	_min_length = options.min_length;
	_max_length = options.max_length;
	for (int i = 0; i < MAX_PASS_LENGTH; i++)
	{
		_thresholds[i] = options.threshold;
	}

	applyLimits(options.limits);

	// Calc length permutations
	_length_permut[0] = 0;
	for (int i = 1; i <= MAX_PASS_LENGTH; i++)
	{
		_length_permut[i] = _length_permut[i - 1] + numOfPermutations(i);
	}

	_length = _min_length;
	_global_index = _length_permut[_min_length - 1];

	// Determine type of markov model to choose statistics
	int stat_type;
	if (options.model == "classic")
		stat_type = _CLASSIC_MARKOV;
	else if (options.model == "layered")
		stat_type = _LAYERED_MARKOV;
	else
		throw invalid_argument("Invalid argument: Unknown Markov model");

	initStat(options.stat_file, stat_type, options.mask);

	// DEBUG
//	printMarkovTable();
}

MarkovPassGen::MarkovPassGen(const MarkovPassGen & o) :
		PassGen(o), _length { o._length }, _global_index { o._global_index }
{
}

MarkovPassGen::~MarkovPassGen()
{
	if (not _generators.empty())
	{
		for (auto i : _generators)
			delete i;

		for (int p = 0; p < MAX_PASS_LENGTH; p++)
			for (int i = 0; i < CHARSET_SIZE; i++)
				delete[] _markov_table[p][i];
	}
}

bool MarkovPassGen::getPassword(char * buffer, uint32_t * length)
{
	// Increment length according to global index
	while (_global_index >= _length_permut[_length])
	{
		_length++;

		if (_length > _max_length) {
			*length = 0;
			return (false);
		}
	}

	// Convert global index to local index
	uint64_t index = _global_index - _length_permut[_length - 1];
	uint64_t partial_index;
	char last_char = 0;
	*length = _length;

	// Create password
	for (int p = 0; p < _length; p++)
	{
		partial_index = index % _thresholds[p];
		index = index / _thresholds[p];

		last_char = _markov_table[p][last_char][partial_index];
		buffer[p] = last_char;
	}

	// Increment index
	_global_index += _step;

	return (true);
}

uint8_t MarkovPassGen::maxPassLen()
{
	// TODO why +1 ???
	return (_max_length + 1);
}

void MarkovPassGen::saveState(std::string filename)
{
	// TODO
}

void MarkovPassGen::loadState(std::string filename)
{
	// TODO
}

void MarkovPassGen::initStat(const std::string & stat_file, int stat_type,
		const std::string & mask)
{
	// Open file with statistics
	ifstream input { stat_file, ifstream::in | ifstream::binary };

	// Find statistics for this generator
	unsigned stat_length = findStat(input, stat_type);

	// Create Markov matrix from statistics
	const unsigned markov_matrix_size = CHARSET_SIZE * CHARSET_SIZE
			* MAX_PASS_LENGTH;
	uint16_t *markov_matrix_buffer = new uint16_t[markov_matrix_size];
	uint16_t *markov_matrix[MAX_PASS_LENGTH][CHARSET_SIZE];
	auto markov_matrix_ptr = markov_matrix_buffer;

	for (int p = 0; p < MAX_PASS_LENGTH; p++)
	{
		for (int j = 0; j < CHARSET_SIZE; j++)
		{
			markov_matrix[p][j] = markov_matrix_ptr;
			markov_matrix_ptr += CHARSET_SIZE;
		}
	}

	// Initialize matrix with values from stat file
	input.read(reinterpret_cast<char *>(markov_matrix_buffer), stat_length);

	// If classic Markov model is set, copy statistics to all positions
	if (stat_type == _CLASSIC_MARKOV)
	{
		for (int p = 1; p < MAX_PASS_LENGTH; p++)
		{
			memcpy(markov_matrix[p], markov_matrix[0],
					CHARSET_SIZE * CHARSET_SIZE * sizeof(uint16_t));
		}
	}

	// Convert these values to host byte order
	for (int i = 0; i < markov_matrix_size; i++)
	{
		markov_matrix_buffer[i] = ntohs(markov_matrix_buffer[i]);
	}

	// Create temporary Markov table, copy values in it and sort them
	MarkovSortTableElement *markov_sort_table_buffer =
			new MarkovSortTableElement[markov_matrix_size];
	MarkovSortTableElement *markov_sort_table[MAX_PASS_LENGTH][CHARSET_SIZE];
	auto markov_sort_table_ptr = markov_sort_table_buffer;

	for (int p = 0; p < MAX_PASS_LENGTH; p++)
	{
		for (int i = 0; i < CHARSET_SIZE; i++)
		{
			markov_sort_table[p][i] = markov_sort_table_ptr;
			markov_sort_table_ptr += CHARSET_SIZE;
		}
	}

	for (int p = 0; p < MAX_PASS_LENGTH; p++)
	{
		for (int i = 0; i < CHARSET_SIZE; i++)
		{
			for (int j = 0; j < CHARSET_SIZE; j++)
			{
				markov_sort_table[p][i][j].next_state = static_cast<uint8_t>(j);
				markov_sort_table[p][i][j].probability = markov_matrix[p][i][j];
			}
		}
	}

	// Apply mask
	applyMask(markov_sort_table, mask);

	// Order elements by probability
	for (int p = 0; p < MAX_PASS_LENGTH; p++)
	{
		for (int i = 0; i < CHARSET_SIZE; i++)
		{
			qsort(markov_sort_table[p][i], CHARSET_SIZE,
					sizeof(MarkovSortTableElement), markovElementCompare);
		}
	}

	// Create final Markov table
	for (int p = 0; p < MAX_PASS_LENGTH; p++)
	{
		unsigned row_size = _thresholds[p];
		for (int i = 0; i < CHARSET_SIZE; i++)
		{
			_markov_table[p][i] = new uint8_t[row_size];
		}
	}

	for (int p = 0; p < MAX_PASS_LENGTH; p++)
		for (int i = 0; i < CHARSET_SIZE; i++)
			for (int j = 0; j < _thresholds[p]; j++)
				_markov_table[p][i][j] = markov_sort_table[p][i][j].next_state;

	delete[] markov_matrix_buffer;
	delete[] markov_sort_table_buffer;
}

bool MarkovPassGen::isFactory()
{
	return (true);
}

PassGen* MarkovPassGen::createGenerator()
{
	_mutex.lock();

	MarkovPassGen *new_generator = new MarkovPassGen { *this };
	_generators.push_back(new_generator);

	// Increment starting index for next generator
	_global_index++;

	_mutex.unlock();

	return (new_generator);
}

void MarkovPassGen::setStep(unsigned step)
{
	_step = step;
}

void MarkovPassGen::printMarkovTable()
{

	for (int p = 0; p < _max_length; p++)
	{
		cout << "========== p = " << p << " ==============\n";

		for (int i = 0; i < CHARSET_SIZE; i++)
		{
			if (not (isValidChar(static_cast<uint8_t>(i)) || i == 0))
				continue;

			cout << "  " << static_cast<char>(i) << " | ";

			for (int j = 0; j < _thresholds[p]; j++)
			{
				cout << static_cast<char>(_markov_table[p][i][j]) << " ";
			}

			cout << "\n";
		}
	}

	cout << "===============================\n";
}

bool MarkovPassGen::isValidChar(uint8_t value)
{
	return ((value >= 32 && value <= 126) ? true : false);
}

unsigned MarkovPassGen::findStat(std::ifstream& stat_file, int stat_type)
{
	// Skip header
	stat_file.ignore(numeric_limits<streamsize>::max(), ETX);

	uint8_t type;
	uint32_t length;

	while (stat_file)
	{
		stat_file >> type;

		stat_file.read(reinterpret_cast<char *>(&length), sizeof(length));
		length = ntohl(length);

		if (type == stat_type)
		{
			return (length);
		}

		stat_file.ignore(length);
	}

	throw runtime_error("Stat file doesn't contain statistics for Markov model");
}

uint64_t MarkovPassGen::numOfPermutations(const unsigned & length)
{
	uint64_t result = 1;

	for (int i = 0; i < length; i++)
	{
		result *= _thresholds[i];
	}

	return (result);
}

void MarkovPassGen::applyMetachar(MarkovSortTableElement** table,
		const char& metachar)
{
	for (int i = 0; i < CHARSET_SIZE; i++)
	{
		for (int j = 0; j < CHARSET_SIZE; j++)
		{
			if (satisfyMask(table[i][j].next_state, metachar))
				table[i][j].probability += UINT16_MAX + 1;
		}
	}
}

void MarkovPassGen::applyChar(MarkovSortTableElement** table,
		const char& character)
{
	for (int i = 0; i < CHARSET_SIZE; i++)
	{
		for (int j = 0; j < CHARSET_SIZE; j++)
		{
			if (table[i][j].next_state == character)
				table[i][j].probability += UINT16_MAX + 1;
		}
	}
}

int MarkovPassGen::markovElementCompare(const void *p1, const void *p2)
{
	const MarkovSortTableElement *e1 =
			static_cast<const MarkovSortTableElement *>(p1);
	const MarkovSortTableElement *e2 =
			static_cast<const MarkovSortTableElement *>(p2);

	if (not isValidChar(e1->next_state))
		return (1);

	return (e2->probability - e1->probability);
}

void MarkovPassGen::applyMask(
		MarkovSortTableElement* table[MAX_PASS_LENGTH][CHARSET_SIZE],
		const std::string& mask)
{
	unsigned position = 0;

	for (int i = 0; i < mask.length(); i++)
	{
		switch (mask[i])
		{
			case '?':
				i++;

				if (mask[i] == '?')
					applyChar(table[position], mask[i]);
				else
					applyMetachar(table[position], mask[i]);

				position++;
				break;
			default:
				applyChar(table[position], mask[i]);

				position++;
				break;
		}

	}
}

bool MarkovPassGen::satisfyMask(uint8_t character, const char& mask)
{
	switch (mask)
	{
		case 'u':
			return ((character >= 65 && character <= 90) ? true : false);
			break;
		case 'l':
			return ((character >= 97 && character <= 122) ? true : false);
			break;
		case 'c':
			return (
					((character >= 65 && character <= 90)
							|| (character >= 97 && character <= 122)) ? true : false);
			break;
		case 'd':
			return ((character >= 48 && character <= 57) ? true : false);
			break;
		case 's':
			return (
					((character >= 32 && character <= 47)
							|| (character >= 58 && character <= 64)
							|| (character >= 91 && character <= 96)
							|| (character >= 123 && character <= 126)) ? true : false);
			break;
		case 'A':
			return (
					((character >= 65 && character <= 90)
							|| (character >= 97 && character <= 122)
							|| (character >= 48 && character <= 57)) ? true : false);
			break;
		case 'a':
			return ((character >= 32 && character <= 126) ? true : false);
			break;
		default:
			return (false);
			break;
	}
}

void MarkovPassGen::applyLimits(const std::string& limits)
{
	if (limits.empty())
		return;

	stringstream ss { limits };
	string value;

	int position = 0;
	while (ss.good() && position <= 64)
	{
		getline(ss, value, ',');
		_thresholds[position] = stoul(value);

		position++;
	}
}
