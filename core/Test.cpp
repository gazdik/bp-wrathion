/*
 * Test.cpp
 *
 *  Created on: Mar 8, 2016
 *      Author: gazdik
 */

#ifdef __WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cstdlib>
#include <getopt.h>

#include <thread>
#include <iostream>

#include "MarkovPassGen.h"
#include "PassGenTest.h"

using namespace std;

struct Arguments : public MarkovPassGenOptions
{
	std::string dictonary;
	bool help = false;
	bool verbose = false;

	PassGen * passgen;
};

const string help = "test [OPTIONS]\n"
		"    -h, --help\t     prints this help\n"
		"    -f, --file\t     dictionary with passwords to crack\n"
		"    -v, --verbose    verbose mode\n"
//"    --threads=NUMTHREADS, -t - number of threads for CPU Cracking\n"
		"\nMarkov attack\n"
		"    --stat=file      stat file for Markov attack\n"
		"    --threshold      number of characters per position (default 5)\n"
		"    --min            minimal length of password (default 1)\n"
		"    --max=value      maximal length of password (default 64)\n";

void generate_password(Arguments & args)
{
	PassGen *passgen;

	// Get instance of generator for this thread
	if (args.passgen->isFactory())
		passgen = args.passgen->createGenerator();
	else
		passgen = args.passgen;

	PassGenTest *test = PassGenTest::CreateInstance();

	char pass_buffer[256];
	uint32_t length;

	while (passgen->getPassword(pass_buffer, &length))
	{
		if (test->TestPassword(pass_buffer, &length) && args.verbose) {
			pass_buffer[length] = 0;
			cout << pass_buffer << endl;
		}
	}
}

int main(int argc, char *argv[])
{
	Arguments args;
	int option, option_index;

	static struct option long_options[] =
	{
			{"help", no_argument, 0, 'h'},
			{"file", required_argument, 0, 'f'},
			{"verbose", no_argument, 0, 'v'},
			{"stat", required_argument, 0, 1},
			{"threshold", required_argument, 0, 2},
			{"min", required_argument, 0, 3},
			{"max", required_argument, 0, 4},
	};

	while ((option = getopt_long(argc, argv, "hf:v", long_options, &option_index))
			!= -1)
	{
		switch (option)
		{
			case 1:
				args.stat_file = optarg;
				break;
			case 2:
				args.threshold = atoi(optarg);
				break;
			case 3:
				args.min_length = atoi(optarg);
				break;
			case 4:
				args.max_length = atoi(optarg);
				break;
			case 'h':
				args.help = true;
				break;
			case 'f':
				args.dictonary = optarg;
				break;
			case 'v':
				args.verbose = true;
				break;
			default:
				return(2);
				break;
		}
	}

	if (args.help)
	{
		cout << help;
		return (0);
	}

	if (args.dictonary.empty()) {
		cerr << "Missing dictionary with passwords" << endl;
		return (2);
	}

	if (args.stat_file.empty()) {
		cerr << "Missing stat file for Markov password generator" << endl;
		return (2);
	}

	// Get number of available processors
	unsigned num_of_cpu = sysconf(_SC_NPROCESSORS_ONLN);

	// Create passgen
	if (not args.stat_file.empty())
	{
		args.passgen = new MarkovPassGen { args };
		args.passgen->setStep(num_of_cpu);
	}

	// Initialize test
	PassGenTest::LoadDictionary(args.dictonary);

	// Initialize threads
	vector<thread *> threads;
	for (int i = 0; i < num_of_cpu; i++)
	{
		threads.push_back(new thread ( generate_password, ref(args) ));
	}

	// Wait for all threads
	for (auto i : threads)
	{
		i->join();
	}

	cout << "Cracked passwords: " << PassGenTest::getNumOfFoundPasswords() << endl;

	return (0);
}

