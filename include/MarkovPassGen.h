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

#include <cstdint>

#include <string>
#include <fstream>
#include <mutex>

const unsigned MIN_PASS_LENGTH = 1;
const unsigned MAX_PASS_LENGTH = 64;
const unsigned DEFAULT_THRESHOLD = 5;
const unsigned CHARSET_SIZE = 256;
const unsigned ETX = 3;

struct MarkovPassGenOptions
{
	std::string stat_file;
	std::string model = "classic";
	unsigned threshold = DEFAULT_THRESHOLD;
	unsigned min_length = MIN_PASS_LENGTH;
	unsigned max_length = MAX_PASS_LENGTH;
	std::string mask;
	std::string limits;
};

class MarkovPassGen : public PassGen
{
public:

	/**
	 * Construct MarkovPassGen factory
	 * @param options Options to init generators
	 */
	MarkovPassGen(MarkovPassGenOptions & options);
	virtual ~MarkovPassGen();

	/**
	 * Generate new password
	 * @param buffer Pointer to char array where to put password
	 * @param length Length of returned password
	 * @return FALSE if there is no password to generate
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
	 * @param filename Path to file with saved state
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
	 * @param o Factory object
	 */
	MarkovPassGen(const MarkovPassGen & o);

private:

	struct MarkovSortTableElement
	{
		uint8_t next_state;
		uint32_t probability;
	};

	const int _CLASSIC_MARKOV = 1;
	const int _LAYERED_MARKOV = 2;

	/**
	 * 3D Markov table
	 */
	static uint8_t *_markov_table[MAX_PASS_LENGTH][CHARSET_SIZE];
	/**
	 * Precomputed permutations for every length
	 */
	static uint64_t _length_permut[MAX_PASS_LENGTH + 1];
	/**
	 * Minimal password length
	 */
	static unsigned _min_length;
	/**
	 * Maximal password length
	 */
	static unsigned _max_length;
	/**
	 * Generator step (ensure exclusivity of passwords)
	 */
	static unsigned _step;
	/**
	 * Number of characters per position
	 */
	static unsigned _thresholds[MAX_PASS_LENGTH];

	static std::mutex _mutex;

	/**
	 * Calc number of permutations for given length and threshold
	 * @param threshold Number of characters per position
	 * @param length Length of password
	 * @return Number of permutations
	 */
	static unsigned numOfPermutations(const unsigned & threshold,
			const unsigned & length);
	/**
	 * Print Markov table
	 */
	void printMarkovTable();
	/**
	 * Read statistics for Markov generator from file, apply mask on them
	 * and initialize Markov tables
	 * @param stat_file path to file
	 */
	void initStat(const std::string & stat_file, int stat_type, const std::string & mask);
	/**
	 * Find data with statistics in stat file
	 * @param stat_file Stream with open stat file
	 * @param stat_type Type of statistics to find
	 * @return Size of statistics in bytes
	 */
	unsigned findStat(std::ifstream& stat_file, int stat_type);

	void applyLimits(const std::string & limits);
	void applyMask(MarkovSortTableElement *table[MAX_PASS_LENGTH][CHARSET_SIZE], const std::string & mask);
	void applyMetachar(MarkovSortTableElement **table, const char & metachar);
	void applyChar(MarkovSortTableElement **table, const char & character);
	/**
	 * Test if the character can be used in password
	 * @param value 8bit character value
	 * @return
	 */
	static bool isValidChar(uint8_t value);
	/**
	 * Compare two elements in Markov sort table
	 * @param p1 First element
	 * @param p2 Second element
	 * @return 0 if elements are equal,
	 * 		<0 if element p2 is less than element p1,
	 * 		>0 otherwise
	 */
	static int markovElementCompare(const void *p1, const void *p2);
	/**
	 * Test if the character satisfy the mask
	 * @param character
	 * @param mask
	 * @return
	 */
	bool satisfyMask(uint8_t character, const char & mask);

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
	 * Instances of generator
	 */
	std::vector<MarkovPassGen *> _generators;

};

#endif /* INCLUDE_MARKOVPASSGEN_H_ */
