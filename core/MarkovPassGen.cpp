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

using namespace std;

unsigned MarkovPassGen::_threshold;
uint8_t * MarkovPassGen::_markov_table_buffer;
uint8_t * MarkovPassGen::_markov_table[MAX_PASS_LENGTH][CHARSET_SIZE];
unsigned MarkovPassGen::_length_permut[MAX_PASS_LENGTH + 1];
unsigned MarkovPassGen::_min_length;
unsigned MarkovPassGen::_max_length;
unsigned MarkovPassGen::_step;
std::mutex MarkovPassGen::_mutex;

MarkovPassGen::MarkovPassGen(MarkovPassGenOptions & options)
{
	_threshold = options.threshold;
	_min_length = options.min_length;
	_max_length = options.max_length;

	// Calc length permutations
	_length_permut[0] = 0;
	for (int i = 1; i <= MAX_PASS_LENGTH; i++)
	{
		_length_permut[i] = _length_permut[i - 1]
				+ numOfPermutations(_threshold, i);
	}

	_next_length = _min_length;
	_next_index = _length_permut[_min_length - 1];

	// Determine type of markov model to choose statistics
	int stat_type;
	if (options.model == "classic")
		stat_type = _CLASSIC_MARKOV;
	else if (options.model == "layered")
		stat_type = _LAYERED_MARKOV;
	else
		throw invalid_argument("Invalid argument: Unknown Markov model");

	readStat(options.stat_file, stat_type);

	// DEBUG
//	printMarkovTable();
}

MarkovPassGen::MarkovPassGen(const MarkovPassGen & o) :
		PassGen(o), _next_length { o._next_length }, _next_index { o._next_index }
{
}

MarkovPassGen::~MarkovPassGen()
{
	if (not _generators.empty())
	{
		delete[] _markov_table_buffer;

		for (auto i : _generators)
			delete i;
	}
}

bool MarkovPassGen::getPassword(char * buffer, uint32_t * length)
{
	if (_exhausted)
	{
		*length = 0;
		return (false);
	}

	unsigned partial_index;
	unsigned next_index = _next_index - _length_permut[_next_length - 1];
	char last_char = 0;

	*length = _next_length;

	// Get first char of password
	partial_index = next_index % _threshold;
	next_index = next_index / _threshold;

	last_char = _markov_table[0][last_char][partial_index];
	buffer[0] = last_char;

	// Get rest of chars
	for (int p = 1; p < _next_length; p++)
	{
		partial_index = next_index % _threshold;
		next_index = next_index / _threshold;

		last_char = _markov_table[p - 1][last_char][partial_index];
		buffer[p] = last_char;
	}

	// Increment index and perhaps length
	_next_index += _step;
	if (_next_index >= _length_permut[_next_length])
	{
		_next_length++;

		if (_next_length > _max_length)
			_exhausted = true;
	}

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

void MarkovPassGen::readStat(std::string & stat_file, int stat_type)
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
	unsigned markov_table_size = MAX_PASS_LENGTH * CHARSET_SIZE * _threshold;
	_markov_table_buffer = new uint8_t[markov_table_size];
	auto markov_table_ptr = _markov_table_buffer;

	for (int p = 0; p < MAX_PASS_LENGTH; p++)
	{
		for (int i = 0; i < CHARSET_SIZE; i++)
		{
			_markov_table[p][i] = markov_table_ptr;
			markov_table_ptr += _threshold;
		}
	}

	for (int p = 0; p < MAX_PASS_LENGTH; p++)
		for (int i = 0; i < CHARSET_SIZE; i++)
			for (int j = 0; j < _threshold; j++)
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
	_next_index++;

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

			for (int j = 0; j < _threshold; j++)
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

unsigned MarkovPassGen::numOfPermutations(const unsigned & threshold,
		const unsigned & length)
{
	unsigned result = 1;

	for (int i = 0; i < length; i++)
	{
		result *= threshold;
	}

	return (result);
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
