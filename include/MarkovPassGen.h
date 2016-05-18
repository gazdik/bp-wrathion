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

#ifndef MARKOVPASSGEN_H_
#define MARKOVPASSGEN_H_

#include "PassGen.h"
#include "Mask.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#else
#include <arpa/inet.h>     // ntohl, ntohs
#endif
#include <string>
#include <ctime>           // timespec
#include <pthread.h>
#include <cstdlib>         // atoi, qsort
#include <cstdint>

class MarkovPassGen : public PassGen
{
public:
  struct Options
  {
    std::string stat_file;
    std::string model = "classic";
    std::string thresholds = "15";
    std::string length = "1:10";
    std::string mask;
  };

  /**
   * Constructor for factory object. This object only initializes memory and
   * creates instances of generators)
   * @param options Command-line options
   * @param cpu_mode If TRUE run generator on CPU instead GPU
   * (also in case a Cracker is running on GPU)
   */
  MarkovPassGen (Options & options, bool cpu_mode = false);
  virtual ~MarkovPassGen ();

  /**
   * Get path to kernel's source code and name of kernel's function
   * @return Pointer to object with these informations ?!?!?!
   */
  virtual KernelCode * getKernelCode();

  /**
   * Set global work-size
   * @param gws
   */
  virtual void setKernelGWS(uint64_t gws);

  /**
   * Initialize OpenCL buffers
   * @param kernel OpenCL kernel
   * @param que OpenCL command-queue
   * @param context OpenCL context
   */
  virtual void initKernel(cl::Kernel *kernel, cl::CommandQueue *que, cl::Context *context);

  /**
   * Initialize kernel's arguments for next step
   * (increments indexes, makes reservation)
   * @return FALSE if all passwords have already been generated
   */
  virtual bool nextKernelStep();

  /**
   * Test if this is only factory for password generator and not generator itself.
   * @return TRUE if it's factory object, FALSE otherwise
   */
  virtual bool isFactory();

  /**
   * Create new instance of the generator
   * @return
   */
  virtual PassGen *createGenerator();

  /**
   * Save current state (NOT IMPLEMENTED because validation of flags in cracker
   * is after incrementing index, i.e. saved index can be ahead)
   * P.S. I'm not sure, but I think the same problem is in old implementation
   * @param filename
   */
  virtual void saveState(std::string filename);

  /**
   * Load and restore saved state of the generator (NOT IMPLEMENTED)
   * @param filename Filename where to save current state
   */
  virtual void loadState(std::string filename);

  /**
   * Get maximum length of password
   * @return
   */
  virtual uint8_t maxPassLen();

  /**
   * Get password by index (only for experiments)
   * @param index
   * @return
   */
  std::string getPassword(uint64_t index);

  /**
   * Get next password
   * @param pass Pointer to array to store the password
   * @param len Length of returned password
   * @return FALSE if all passwords have been generated
   */
  virtual bool getPassword(char* pass, uint32_t *len);
private:

  /**
   * Construct new instance of generator from factory object
   */
  MarkovPassGen (const MarkovPassGen & o);

  /**
   * Element for sorting Markov's charset (character with probability)
   */
  struct SortElement
  {
    uint8_t next_state;
    uint32_t probability;
  };

  /**
   * Supported types of Markov model
   */
  enum Model
  {
    CLASSIC = 1, LAYERED = 2
  };

  /**
   * Delimiter between text header and binary data in stat file
   */
  const unsigned ETX = 3;
  /**
   * Path to kernel's source
   */
  const std::string _kernel_source = "kernels/markov_passgen.cl";
  /**
   * Name of kernel's function
   */
  const std::string _kernel_name = "markov_passgen";
  /**
   * ID of factory object (generators have ID from 1 to number of generators)
   */
  const int FACTORY_INSTANCE_ID = -1;

  /**
   * Initialize Markov's charset, ...
   * @param stat_file
   */
  void initMemory(std::string stat_file);

  /**
   * Reserve new set of indexes
   * @return TRUE if successful, FALSE otherwise
   */
  bool reservePasswords();

  /**
   * Parse command line options
   * @param options Command line options
   */
  void parseOptions(Options & options);

  /**
   * Compare probabilities of two characters
   * @param p1
   * @param p2
   * @return values to pass into C sort function
   */
  static int compareSortElements(const void *p1, const void *p2);

  /**
   * Test if it's valid password character
   * (in the range of 32-255 - includes extended ASCII)
   * @param value
   * @return TRUE if it's valid character
   */
  static bool isValidChar(uint8_t value);

  /**
   * Calc total number of password combinations for given length
   * @param length
   * @return
   */
  uint64_t numPermutations(unsigned length);

  /**
   * Find binary data in file with statistics and set stream to first byte
   * @param stat_file
   * @return size of binary data
   */
  unsigned findStatistics(std::ifstream & stat_file);

  /**
   * Apply mask by adjusting character's probabilities
   * @param table
   */
  void applyMask(SortElement *table[MAX_PASS_LENGTH][ASCII_CHARSET_SIZE]);

  /**
   * Print debug informations
   */
  void debugPrint();

  /**
   * Print verbose informations
   */
  void verbosePrint();

  // only for testing
  static bool _cpu_mode;

  static KernelCode _gpu_code;
  static Mask _mask;
  static Model _model;
  static cl_uchar *_markov_table;
  static std::size_t _markov_table_size;
  static cl_ulong *_permutations;
  static cl_uint _min_length;
  static cl_uint _max_length;
  static cl_uint *_thresholds;
  static cl_uint _max_threshold;

  static int _num_instances;
  static cl_ulong _shared_start_index;
  static cl_ulong _shared_stop_index;
  static pthread_mutex_t _shared_index_mutex;
  static pthread_mutexattr_t _shared_index_mutex_attr;

  cl_ulong _private_start_index = 1;
  cl_ulong _private_stop_index = 0;
  unsigned _min_reservation_size;
  unsigned _reservation_size;

  std::size_t _gws = 256;
  struct timespec _speed_clock;
  // Current length
  cl_uint _length = 1;

  int _instance_id;
  std::vector<MarkovPassGen *> _instances;

  cl::Kernel _kernel;
  cl::Buffer _markov_table_buffer;
  cl::Buffer _thresholds_buffer;
  cl::Buffer _permutations_buffer;
};

#endif /* MARKOVPASSGEN_H_ */
