/*
 * PassGenTest.h
 *
 *  Created on: Mar 7, 2016
 *      Author: gazdik
 */

#ifndef CORE_PASSGENTEST_H_
#define CORE_PASSGENTEST_H_

#include <unordered_set>
#include <string>
#include <vector>
#include <mutex>

const unsigned BUFFER_SIZE = 256;

/**
 * Class for testing password generator to provide the information about
 * success rate of generator against plaintext passwords in some dictionary.
 */
class PassGenTest
{
public:
	virtual ~PassGenTest();

	/**
	 * Create new instance of this class
	 * @return
	 */
	static PassGenTest * CreateInstance();

	/**
	 * Delete all previous created instances
	 */
	static void DeleteInstances();


	/**
	 * Load dictionary with passwords to crack into memory
	 * @param path
	 */
	static void LoadDictionary(std::string & path);

	/**
	 * Test if the password is in dictionary
	 * @param pass
	 * @param length
	 */
	bool TestPassword(const char *pass, const uint32_t *length);

	/**
	 * Return number of cracked passwords
	 */
	static unsigned getNumCrackedPasswords();


protected:
	PassGenTest();

private:
	static unsigned _cnt_cracked_passwords;
	static std::unordered_set<std::string> _dictionary;
	static std::vector<PassGenTest *> _instances;
	static std::mutex _mutex;
	char _line_buffer[BUFFER_SIZE];
};

#endif /* CORE_PASSGENTEST_H_ */
