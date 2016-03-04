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

#ifndef INCLUDE_MARKOVPASSGEN_H_
#define INCLUDE_MARKOVPASSGEN_H_

#include "PassGen.h"

#include <arpa/inet.h>
#include <cstdint>
#include <cstdlib>

#include <string>
#include <fstream>
#include <limits>
#include <stdexcept>


const unsigned MIN_PASS_LENGTH = 1;
const unsigned MAX_PASS_LENGTH = 64;
const unsigned DEFAULT_THRESHOLD = 5;
const unsigned CHARSET_SIZE = 256;
const unsigned ETX = 3;

struct MarkovPassGenOptions
{
	std::string stat_file;
	unsigned threshold = DEFAULT_THRESHOLD;
	unsigned min_length = MIN_PASS_LENGTH;
	unsigned max_length = MAX_PASS_LENGTH;
};

struct MarkovSortTableElement
{
	uint8_t next_state;
	uint16_t probability;
};

class MarkovPassGen : public PassGen
{
public:
	MarkovPassGen(MarkovPassGenOptions & options);
	virtual ~MarkovPassGen();
	/**
	 * Get next password
	 * @param buffer pointer to char array where to put password
	 * @param length length of returned password
	 * @return false if this is last password
	 */
	virtual bool getPassword(char *buffer, uint32_t *length);

	/**
	 * Return maximum password length
	 * @return
	 */
	virtual uint8_t maxPassLen();

	/**
	 * Save current state of generator
	 * @param filename path to file where to save state
	 */
	virtual void saveState(std::string filename);

	/**
	 * Restore generator state from file
	 * @param filename path to file with save state
	 */
	virtual void loadState(std::string filename);

protected:

	/**
	 * Read statistics for Markov generator from file
	 * and initialize Markov tables
	 * @param stat_file path to file
	 */
	void readStat(std::string & stat_file);

	/**
	 * Find data with statistics in stat file
	 * @param stat_file input stream
	 */
	void findStat(std::ifstream& stat_file);

	/**
	 * Number of characters per position
	 */
	unsigned _threshold;

	uint8_t *_markov_table_buffer;
	uint8_t *_markov_table[CHARSET_SIZE];

	/**
	 * Precomputed permutations for every length
	 */
	unsigned _length_permut[MAX_PASS_LENGTH + 1];

	unsigned _min_length;
	unsigned _max_length;
	unsigned _curr_length;
	unsigned _curr_index;
	bool _exhausted = false;

	const unsigned _STAT_TYPE = 1;

	/**
	 * Calc number of permutations for given length and threshold
	 * @param threshold number of char per position
	 * @param length length of password
	 * @return number of permutations
	 */
	static unsigned calcPermutations(const unsigned &threshold,
			const unsigned & length);

	// TODO
//	static void calc_permutations(mpz_class & result, const mpz_class &threshold,
//			const unsigned long int & length);

	void printMarkovTable();
};

/**
 * Compare two elements in markov_sort_table
 * @param p1
 * @param p2
 * @return 0 if elements are equal,
 * 		<0 if element p2 is less than element p1,
 * 		>0 if element p1 is less then element p2
 */
int markovElementCompare(const void *p1, const void *p2);

	/**
	 * Test, if the character can be used in password
	 * @param value 8bit character
	 * @return true if it's valid character
	 */
	static bool isValidChar(uint8_t value);


#endif /* INCLUDE_MARKOVPASSGEN_H_ */
