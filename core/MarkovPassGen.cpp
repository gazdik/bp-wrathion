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

#include <iostream>

using namespace std;

MarkovPassGen::MarkovPassGen(MarkovPassGenOptions& options) :
		_threshold { options.threshold }, _min_length { options.min_length }, _max_length {
				options.max_length }
{
	// calc length permutations
	_length_permut[0] = 0;
	for (unsigned i = 1; i <= MAX_PASS_LENGTH; i++)
	{
		_length_permut[i] = _length_permut[i - 1]
				+ calcPermutations(_threshold, i);
	}

	_curr_length = _min_length;
	_curr_index = _length_permut[_min_length - 1];

	readStat(options.stat_file);

	printMarkovTable();
}

MarkovPassGen::~MarkovPassGen()
{
	delete[] _markov_table_buffer;
}

bool MarkovPassGen::getPassword(char* buffer, uint32_t* length)
{
	char tmp_buffer[MAX_PASS_LENGTH + 1];

	if (_exhausted)
		return false;

	unsigned partial_index;
	unsigned next_index = _curr_index - _length_permut[_curr_length - 1];
	uint8_t last_char = 0;
	*length = _curr_length;

	// get current password
	for (unsigned i = 0; i < _curr_length; i++)
	{
		partial_index = next_index % _threshold;
		next_index = next_index / _threshold;

		last_char = _markov_table[last_char][partial_index];
		buffer[i] = last_char;
	}

	// Increment index and perhaps length
	_curr_index++;
	if (_curr_index >= _length_permut[_curr_length])
	{
		_curr_length++;

		if (_curr_length > _max_length)
			_exhausted = true;
	}

	return true;
}

uint8_t MarkovPassGen::maxPassLen()
{
	return _max_length;
}

void MarkovPassGen::saveState(std::string filename)
{
	// TODO
}

void MarkovPassGen::loadState(std::string filename)
{
	// TODO
}

void MarkovPassGen::readStat(std::string& stat_file)
{
	// Open file with statistics
	ifstream input { stat_file, ifstream::in | ifstream::binary };

	// Find statistics for this generator
	findStat(input);

	// Create Markov matrix from statistics
	const unsigned markov_matrix_size = CHARSET_SIZE * CHARSET_SIZE;
	uint16_t *markov_matrix_buffer = new uint16_t[markov_matrix_size];
	uint16_t *markov_matrix[CHARSET_SIZE];
	auto markov_matrix_ptr = markov_matrix_buffer;

	for (unsigned i = 0; i < CHARSET_SIZE; i++)
	{
		markov_matrix[i] = markov_matrix_ptr;
		markov_matrix_ptr += CHARSET_SIZE;
	}

	// Initialize matrix with values from stat file
	input.read(reinterpret_cast<char *>(markov_matrix_buffer),
			markov_matrix_size * sizeof(uint16_t));

	// Convert these values to host byte order
	for (unsigned i = 0; i < markov_matrix_size; i++)
	{
		markov_matrix_buffer[i] = ntohs(markov_matrix_buffer[i]);
	}

	// Create temporary Markov table, copy values in it and sort them
	MarkovSortTableElement *markov_sort_table_buffer =
			new MarkovSortTableElement[markov_matrix_size];
	MarkovSortTableElement *markov_sort_table[CHARSET_SIZE];
	auto markov_sort_table_ptr = markov_sort_table_buffer;

	for (unsigned i = 0; i < CHARSET_SIZE; i++)
	{
		markov_sort_table[i] = markov_sort_table_ptr;
		markov_sort_table_ptr += CHARSET_SIZE;
	}

	for (unsigned i = 0; i < CHARSET_SIZE; i++)
	{
		for (unsigned j = 0; j < CHARSET_SIZE; j++)
		{
			markov_sort_table[i][j].next_state = static_cast<uint8_t>(j);
			markov_sort_table[i][j].probability = markov_matrix[i][j];
		}
	}

	// Order elements by probability
	for (unsigned i = 0; i < CHARSET_SIZE; i++)
	{
		qsort(markov_sort_table[i], CHARSET_SIZE, sizeof(MarkovSortTableElement),
				markovElementCompare);
	}

	// Create final Markov table
	unsigned markov_table_size = CHARSET_SIZE * _threshold;
	_markov_table_buffer = new uint8_t[markov_table_size];
	auto markov_table_ptr = _markov_table_buffer;

	for (unsigned i = 0; i < CHARSET_SIZE; i++)
	{
		_markov_table[i] = markov_table_ptr;
		markov_table_ptr += _threshold;
	}

	for (unsigned i = 0; i < CHARSET_SIZE; i++)
		for (unsigned j = 0; j < _threshold; j++)
			_markov_table[i][j] = markov_sort_table[i][j].next_state;

	delete[] markov_matrix_buffer;
	delete[] markov_sort_table_buffer;
}

void MarkovPassGen::printMarkovTable()
{
	for (unsigned i = 0; i < CHARSET_SIZE; i++)
	{
		if (not (isValidChar(static_cast<uint8_t>(i)) || i == 0))
			continue;

		cout << "Current state: " << static_cast<char>(i) << "\n"
				<< "    Next states: ";

		for (unsigned j = 0; j < _threshold; j++)
		{
			cout << static_cast<char>(_markov_table[i][j]) << " ";
		}

		cout << "\n";
	}
}

bool isValidChar(uint8_t value)
{
	return ((value >= 32 && value <= 126) ? true : false);
}

void MarkovPassGen::findStat(std::ifstream& stat_file)
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

		if (type == _STAT_TYPE)
			return;

		stat_file.ignore(length);
	}

	throw runtime_error("Stat file doesn't contain statistics for Markov model");
}

unsigned MarkovPassGen::calcPermutations(const unsigned & threshold,
		const unsigned & length)
{
	unsigned result = 1;

	for (unsigned i = 0; i < length; i++)
	{
		result *= threshold;
	}

	return (result);
}

int markovElementCompare(const void *p1, const void *p2)
{
	const MarkovSortTableElement *e1 =
			static_cast<const MarkovSortTableElement *>(p1);
	const MarkovSortTableElement *e2 =
			static_cast<const MarkovSortTableElement *>(p2);

	if (not isValidChar(e1->next_state))
		return (1);

	return (e2->probability - e1->probability);
}

//void MarkovPassGen::calc_permutations(mpz_class& result,
//		const mpz_class& threshold, const unsigned long int& length)
//{
//	mpz_pow_ui(result.get_mpz_t(), threshold.get_mpz_t(), length);
//}
