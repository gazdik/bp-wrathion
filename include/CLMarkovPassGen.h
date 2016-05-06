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

#ifndef CLMARKOVPASSGEN_H_
#define CLMARKOVPASSGEN_H_

#include "PassGen.h"
#include "Mask.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>     // ntohl, ntohs
#endif
#include <string>
#include <ctime>           // timespec
#include <pthread.h>
#include <cstdlib>         // atoi, qsort
#include <cstdint>

class CLMarkovPassGen : public PassGen
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

  CLMarkovPassGen (Options & options);
  virtual ~CLMarkovPassGen ();

  virtual KernelCode * getKernelCode();
  virtual void setKernelGWS(uint64_t gws);
  virtual void initKernel(cl::Kernel *kernel, cl::CommandQueue *que, cl::Context *context);
  virtual bool nextKernelStep();

  virtual bool isFactory();
  virtual PassGen *createGenerator();
  // TODO Implement ???
//  virtual void saveState(std::string filename);
//  virtual void loadState(std::string filename);
  virtual uint8_t maxPassLen();
  std::string getPassword(uint64_t index);
private:

  CLMarkovPassGen (const CLMarkovPassGen & o);

  struct SortElement
  {
    uint8_t next_state;
    uint32_t probability;
  };

  enum Model
  {
    CLASSIC = 1, LAYERED = 2
  };

  const unsigned ETX = 3;
  const std::string _kernel_source = "kernels/markov_passgen.cl";
  const std::string _kernel_name = "markov_passgen";
  const int FACTORY_INSTANCE_ID = -1;

  void initMemory(std::string stat_file);
  bool reservePasswords();
  void parseOptions(Options & options);
  static int compareSortElements(const void *p1, const void *p2);
  static bool isValidChar(uint8_t value);
  uint64_t numPermutations(unsigned length);
  unsigned findStatistics(std::ifstream & stat_file);
  void applyMask(SortElement *table[MAX_PASS_LENGTH][ASCII_CHARSET_SIZE]);

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
  static cl_ulong _global_start_index;
  static cl_ulong _global_stop_index;

  static pthread_mutex_t _global_index_mutex;
  static pthread_mutexattr_t _global_index_mutex_attr;

  std::size_t _gws;
  cl_ulong _local_start_index = 1;
  cl_ulong _local_stop_index = 0;
  std::size_t _reservation_size;
  struct timespec _speed_clock;

  cl::Kernel _kernel;
  cl::Buffer _markov_table_buffer;
  cl::Buffer _thresholds_buffer;
  cl::Buffer _permutations_buffer;

  int _instance_id = FACTORY_INSTANCE_ID;
  std::vector<CLMarkovPassGen *> _instances;
};

#endif /* CLMARKOVPASSGEN_H_ */
