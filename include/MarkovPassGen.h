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

	/**
	 * Construct MarkovPassGen factory
	 * @param options
	 */
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

	/**
	 * Return true if this object is only factory
	 * for password generator and not generator itself
	 * @return
	 */
	virtual bool isFactory();

	/**
	 * Create new generator
	 * @return
	 */
	virtual PassGen* createGenerator();

	/**
	 * Set step for generators (to ensure exclusivity of passwords)
	 * @param step
	 */
	virtual void setStep(unsigned step);

protected:

	/**
	 * Construct generator object based on factory object
	 * @param o factory object
	 */
	MarkovPassGen(const MarkovPassGen & o);

	/**
	 * Calc number of permutations for given length and threshold
	 * @param threshold number of char per position
	 * @param length length of password
	 * @return number of permutations
	 */
	static unsigned numOfPermutations(const unsigned &threshold,
			const unsigned & length);

	/**
	 * Print Markov table
	 */
	void printMarkovTable();

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
	static unsigned _threshold;

	/**
	 * Markov table buffer
	 */
	static uint8_t *_markov_table_buffer;

	/**
	 * 2D Markov table
	 */
	static uint8_t *_markov_table[CHARSET_SIZE];

	/**
	 * Precomputed permutations for every length
	 */
	static unsigned _length_permut[MAX_PASS_LENGTH + 1];

	/**
	 * Minimal password length
	 */
	static unsigned _min_length;

	/**
	 * Maximal password length
	 */
	static unsigned _max_length;

	/**
	 * Length of next password
	 */
	unsigned _next_length;

	/**
	 * Index of next password
	 */
	unsigned _next_index;

	/**
	 * Indication of end
	 */
	bool _exhausted = false;

	/**
	 * Generator step (ensure exclusivity of passwords)
	 */
	static unsigned _step;

	/**
	 * Instances of generator
	 */
	std::vector<MarkovPassGen *> _generators;

	const unsigned _STAT_TYPE = 1;

};

/**
 * Compare two elements in Markov sort table
 * @param p1
 * @param p2
 * @return 0 if elements are equal,
 * 		<0 if element p2 is less than element p1,
 * 		>0 if element p1 is less then element p2
 */
int markovElementCompare(const void *p1, const void *p2);

/**
 * Test if the character can be used in password
 * @param value 8bit character
 * @return
 */
static bool isValidChar(uint8_t value);

#endif /* INCLUDE_MARKOVPASSGEN_H_ */
