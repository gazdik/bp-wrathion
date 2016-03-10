/*
 * PassGenTest.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: gazdik
 */

#include "PassGenTest.h"

#include <fstream>
#include <cstring>

#include <thread>
#include <iostream>

using namespace std;

unsigned PassGenTest::_cnt_found_passwords = 0;
std::unordered_set<std::string> PassGenTest::_dictionary;
std::vector<PassGenTest *> PassGenTest::_instances;
std::mutex PassGenTest::_mutex;

PassGenTest* PassGenTest::CreateInstance()
{
	PassGenTest *instance = new PassGenTest { };

	_instances.push_back(instance);

	return (instance);
}

void PassGenTest::DeleteInstances()
{
	for (auto i : _instances)
		delete i;

	_instances.clear();
}

void PassGenTest::LoadDictionary(std::string& path)
{
	ifstream input { path, ifstream::in };

	string password;

 	while (input)
	{
		getline(input, password);

		_dictionary.insert(string { password });
	}
}

bool PassGenTest::TestPassword(const char* pass, const uint32_t* length)
{
	memcpy(_line_buffer, pass, *length);
	_line_buffer[*length] = 0;

	string password { _line_buffer };

	if (_dictionary.count(password) != 0) {
		_mutex.lock();
		_cnt_found_passwords++;
		_mutex.unlock();

		return (true);
	}
	return (false);
}

unsigned PassGenTest::getNumOfFoundPasswords()
{
	return (_cnt_found_passwords);
}

PassGenTest::PassGenTest()
{
}

PassGenTest::~PassGenTest()
{
}

