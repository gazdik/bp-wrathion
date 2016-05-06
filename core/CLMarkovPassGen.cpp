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

#include "CLMarkovPassGen.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <arpa/inet.h>     // ntohl, ntohs
#endif

#include <algorithm>        // max_element
#include <limits>
#include <sstream>          // stringstream
#include <iostream> // TODO

using namespace std;

PassGen::KernelCode CLMarkovPassGen::_gpu_code;
Mask CLMarkovPassGen::_mask;
CLMarkovPassGen::Model CLMarkovPassGen::_model;
cl_uchar * CLMarkovPassGen::_markov_table;
std::size_t CLMarkovPassGen::_markov_table_size;
cl_ulong * CLMarkovPassGen::_permutations;
cl_uint CLMarkovPassGen::_min_length;
cl_uint CLMarkovPassGen::_max_length;
int CLMarkovPassGen::_num_instances;
cl_uint * CLMarkovPassGen::_thresholds;
cl_uint CLMarkovPassGen::_max_threshold;
cl_ulong CLMarkovPassGen::_global_start_index;
cl_ulong CLMarkovPassGen::_global_stop_index;
pthread_mutex_t CLMarkovPassGen::_global_index_mutex;
pthread_mutexattr_t CLMarkovPassGen::_global_index_mutex_attr;

CLMarkovPassGen::CLMarkovPassGen(CLMarkovPassGen::Options & options)
{
  pthread_mutexattr_init(&_global_index_mutex_attr);
  pthread_mutex_init(&_global_index_mutex, &_global_index_mutex_attr);
  _mask = Mask { options.mask };
  _thresholds = new cl_uint[MAX_PASS_LENGTH];
  _permutations = new cl_ulong[MAX_PASS_LENGTH + 1];

  parseOptions(options);

  // Determine maximal threshold
  _max_threshold = *max_element(_thresholds, _thresholds + MAX_PASS_LENGTH);

  // Initialize memory
  initMemory(options.stat_file);

  _global_start_index = _permutations[_min_length - 1];
  _global_stop_index = _permutations[_max_length];
//  _reservation_size = _gws; // The lowest value

  _num_instances = 0;

  _gpu_code.filename = _kernel_source;
  _gpu_code.name = _kernel_name;

#ifndef NDEBUG
  cout << "Minimal length: " << _min_length << "\n";
  cout << "Maximal length: " << _max_length << "\n";
#endif

  cout << "Thresholds: ";
  for (int i = 0; i < _max_length; i++)
  {
    cout << _thresholds[i] << " ";
  }
  cout << "\n";

#ifndef NDEBUG
  cout << "Maximal threshold: " << _max_threshold << "\n";

  cout << "Model: ";
  if (_model == Model::CLASSIC)
    cout << "classic";
  else if (_model == Model::LAYERED)
    cout << "layered";
  cout << "\n";
#endif
}

CLMarkovPassGen::CLMarkovPassGen(const CLMarkovPassGen& o) :
    _instance_id { o._num_instances++ }
{
}

CLMarkovPassGen::~CLMarkovPassGen()
{

  if (_instance_id == FACTORY_INSTANCE_ID)
  {
    pthread_mutex_destroy(&_global_index_mutex);
    pthread_mutexattr_destroy(&_global_index_mutex_attr);

    for (auto i : _instances)
      delete i;

    delete[] _thresholds;
    delete[] _permutations;
  }
}

PassGen::KernelCode* CLMarkovPassGen::getKernelCode()
{
  return &_gpu_code;
}

bool CLMarkovPassGen::isFactory()
{
  return (_instance_id == FACTORY_INSTANCE_ID);
}

PassGen* CLMarkovPassGen::createGenerator()
{
  if (_instance_id == FACTORY_INSTANCE_ID)
  {
    CLMarkovPassGen * new_instance = new CLMarkovPassGen { *this };
    _instances.push_back(new_instance);
    return (new_instance);
  }

  return (nullptr);
}

uint8_t CLMarkovPassGen::maxPassLen()
{
  return (static_cast<uint8_t>(_max_length));
}

void CLMarkovPassGen::setKernelGWS(uint64_t gws)
{
  _gws = gws;
  // Initialize reservation size
  _min_reservation_size = 4 * _gws;
}

void CLMarkovPassGen::initKernel(cl::Kernel* kernel, cl::CommandQueue* que,
                                 cl::Context* context)
{
  _kernel = *kernel;

  // Invalid values to prevent kernel execution without reserved passwords
  _local_start_index = 1;
  _local_stop_index = 0;

  _markov_table_buffer = cl::Buffer { *context, CL_MEM_READ_ONLY,
                                      _markov_table_size * sizeof(cl_uchar) };
  que->enqueueWriteBuffer(_markov_table_buffer, CL_FALSE, 0,
                          _markov_table_size * sizeof(cl_uchar), _markov_table);

  _thresholds_buffer = cl::Buffer { *context, CL_MEM_READ_ONLY,
                                    _max_length * sizeof(cl_uint) };
  que->enqueueWriteBuffer(_thresholds_buffer, CL_FALSE, 0,
                          _max_length * sizeof(cl_uint), _thresholds);

  _permutations_buffer = cl::Buffer { *context, CL_MEM_READ_ONLY,
                                      (_max_length + 2) * sizeof(cl_ulong) };
  que->enqueueWriteBuffer(_permutations_buffer, CL_FALSE, 0,
                          (_max_length + 2) * sizeof(cl_ulong), _permutations);

  kernel->setArg(2, _markov_table_buffer);
  kernel->setArg(3, _thresholds_buffer);
  kernel->setArg(4, _permutations_buffer);
  kernel->setArg(5, _max_threshold);
  kernel->setArg(6, _local_start_index);
  kernel->setArg(7, _local_stop_index);
}

bool CLMarkovPassGen::nextKernelStep()
{
  if (_local_start_index < _local_stop_index)
  {
    _local_start_index += _gws;
    _kernel.setArg(6, _local_start_index);
    return (true);
  }

  if (reservePasswords())
  {
    _kernel.setArg(6, _local_start_index);
    _kernel.setArg(7, _local_stop_index);
    return (true);
  }

  return (false);
}

bool CLMarkovPassGen::reservePasswords()
{
  if (_local_start_index == 0)
  {
    _reservation_size = _min_reservation_size;
    clock_gettime(CLOCK_MONOTONIC, &_speed_clock);
  }
  else
  {
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - _speed_clock.tv_sec);
    //elapsed += (end.tv_nsec - _speed_clock.tv_nsec) / 1000000000.0;

    unsigned speed = _reservation_size / elapsed;
    _speed_clock = end;
	unsigned new_res_size = speed;

    unsigned max_res_size = _reservation_size << 5;		// Multiply by 16
    if (new_res_size > max_res_size)
    {
      new_res_size = max_res_size;
    }
    _reservation_size = new_res_size;

    if (_reservation_size < _min_reservation_size)
      _reservation_size = _min_reservation_size;
  }

  pthread_mutex_lock(&_global_index_mutex);
  _local_start_index = _global_start_index;
  _global_start_index += _reservation_size;
  pthread_mutex_unlock(&_global_index_mutex);

  _local_stop_index = _local_start_index + _reservation_size;

  if (_local_start_index > _global_stop_index)
    return (false);

  if (_local_stop_index > _global_stop_index)
    _local_stop_index = _global_stop_index;

  return (true);
}

void CLMarkovPassGen::initMemory(std::string stat_file)
{
  // Calculate permutations for all lengths
  _permutations[0] = 0;
  for (int i = 1; i < MAX_PASS_LENGTH + 1; i++)
  {
    _permutations[i] = _permutations[i - 1] + numPermutations(i);
  }

  // Open file with statistics and find appropriate statistics
  ifstream input { stat_file, ifstream::in | ifstream::binary };
  unsigned stat_length = findStatistics(input);

  // Create Markov matrix from statistics
  const unsigned markov_matrix_size = ASCII_CHARSET_SIZE * ASCII_CHARSET_SIZE
      * MAX_PASS_LENGTH;
  uint16_t *markov_matrix_buffer = new uint16_t[markov_matrix_size];
  uint16_t *markov_matrix[MAX_PASS_LENGTH][ASCII_CHARSET_SIZE];
  auto markov_matrix_ptr = markov_matrix_buffer;

  for (int p = 0; p < MAX_PASS_LENGTH; p++)
  {
    for (int i = 0; i < ASCII_CHARSET_SIZE; i++)
    {
      markov_matrix[p][i] = markov_matrix_ptr;
      markov_matrix_ptr += ASCII_CHARSET_SIZE;
    }
  }

  input.read(reinterpret_cast<char *>(markov_matrix_buffer), stat_length);

  // In case of classic Markov model, copy statistics to all positions
  if (_model == Model::CLASSIC)
  {
    markov_matrix_ptr = markov_matrix_buffer;
    for (int p = 1; p < MAX_PASS_LENGTH; p++)
    {
      markov_matrix_ptr += ASCII_CHARSET_SIZE * ASCII_CHARSET_SIZE;

      memcpy(markov_matrix_ptr, markov_matrix_buffer,
             ASCII_CHARSET_SIZE * ASCII_CHARSET_SIZE * sizeof(uint16_t));
    }
  }

  // Convert these values to host byte order
  for (int i = 0; i < markov_matrix_size; i++)
  {
    markov_matrix_buffer[i] = ntohs(markov_matrix_buffer[i]);
  }

  // Create temporary Markov table to order elements
  SortElement *markov_sort_table_buffer = new SortElement[markov_matrix_size];
  SortElement *markov_sort_table[MAX_PASS_LENGTH][ASCII_CHARSET_SIZE];
  auto markov_sort_table_ptr = markov_sort_table_buffer;

  for (int p = 0; p < MAX_PASS_LENGTH; p++)
  {
    for (int i = 0; i < ASCII_CHARSET_SIZE; i++)
    {
      markov_sort_table[p][i] = markov_sort_table_ptr;
      markov_sort_table_ptr += ASCII_CHARSET_SIZE;
    }
  }

  for (int p = 0; p < MAX_PASS_LENGTH; p++)
  {
    for (int i = 0; i < ASCII_CHARSET_SIZE; i++)
    {
      for (int j = 0; j < ASCII_CHARSET_SIZE; j++)
      {
        markov_sort_table[p][i][j].next_state = static_cast<uint8_t>(j);
        markov_sort_table[p][i][j].probability = markov_matrix[p][i][j];
      }
    }
  }

  // Apply mask
  applyMask(markov_sort_table);

  // Order elements by probability
  for (int p = 0; p < MAX_PASS_LENGTH; p++)
  {
    for (int i = 0; i < ASCII_CHARSET_SIZE; i++)
    {
      qsort(markov_sort_table[p][i], ASCII_CHARSET_SIZE, sizeof(SortElement),
            compareSortElements);
    }
  }

  // Create final Markov table
  _markov_table_size = _max_length * ASCII_CHARSET_SIZE * _max_threshold;
  _markov_table = new cl_uchar[_markov_table_size];

  unsigned index, index1, index2;
  for (unsigned p = 0; p < _max_length; p++)
  {
    index1 = p * ASCII_CHARSET_SIZE * _max_threshold;
    for (unsigned i = 0; i < ASCII_CHARSET_SIZE; i++)
    {
      index2 = i * _max_threshold;
      for (unsigned j = 0; j < _max_threshold; j++)
      {
        index = index1 + index2 + j;
        _markov_table[index] = markov_sort_table[p][i][j].next_state;
      }
    }
  }

  delete[] markov_matrix_buffer;
  delete[] markov_sort_table_buffer;
}

void CLMarkovPassGen::parseOptions(CLMarkovPassGen::Options & options)
{
  stringstream ss;
  string substr;

  // Parse length values
  ss << options.length;

  std::getline(ss, substr, ':');
  _min_length = stoi(substr);

  std::getline(ss, substr);
  _max_length = stoi(substr);

  // Parse threshold values
  ss.clear();
  ss << options.thresholds;

  // Set global threshold
  std::getline(ss, substr, ':');
  for (int i = 0; i < MAX_PASS_LENGTH; i++)
  {
    _thresholds[i] = stoi(substr);
  }

  // Adjust threshold values according to mask
  for (int i = 0; i < MAX_PASS_LENGTH; i++)
  {
    unsigned mask_chars_count = _mask[i].Count();

    if (mask_chars_count < _thresholds[i])
      _thresholds[i] = mask_chars_count;
  }

  // Set positional thresholds
  int i = 0;
  while (std::getline(ss, substr, ','))
  {
    _thresholds[i] = stoi(substr);
    i++;
  }

  // Parse model
  if (options.model == "classic")
    _model = Model::CLASSIC;
  else if (options.model == "layered")
    _model = Model::LAYERED;
  else
    throw invalid_argument("Invalid value for argument 'model'");
}

int CLMarkovPassGen::compareSortElements(const void *p1, const void *p2)
{
  const SortElement *e1 = static_cast<const SortElement *>(p1);
  const SortElement *e2 = static_cast<const SortElement *>(p2);

  if (!isValidChar(e1->next_state) && isValidChar(e2->next_state))
    return (1);

  if (isValidChar(e1->next_state) && !isValidChar(e2->next_state))
	  return (-1);

  if (!isValidChar(e1->next_state) && !isValidChar(e2->next_state))
    return (e2->next_state - e1->next_state);

  int result = e2->probability - e1->probability;

  if (result == 0)
    return (e2->next_state - e1->next_state);

  return (result);
}

bool CLMarkovPassGen::isValidChar(uint8_t value)
{
  return ((value >= 32) ? true : false);
}


uint64_t CLMarkovPassGen::numPermutations(unsigned length)
{
  uint64_t result = 1;
  for (unsigned i = 0; i < length; i++)
  {
    result *= _thresholds[i];
  }

  return (result);
}

unsigned CLMarkovPassGen::findStatistics(std::ifstream & stat_file)
{
  // Skip header
  stat_file.ignore(numeric_limits<streamsize>::max(), ETX);

  uint8_t type;
  uint32_t length;

  while (stat_file)
  {
    stat_file.read(reinterpret_cast<char *>(&type), sizeof(type));
    stat_file.read(reinterpret_cast<char *>(&length), sizeof(length));
    length = ntohl(length);

    if (type == _model)
    {
      return (length);
    }

    stat_file.ignore(length);
  }

  throw runtime_error {
      "File doesn't contain statistics for specified Markov model" };
}

void CLMarkovPassGen::applyMask(CLMarkovPassGen::SortElement *table[MAX_PASS_LENGTH][ASCII_CHARSET_SIZE])
{
  for (unsigned p = 0; p < _max_length; p++)
  {
    for (unsigned i = 0; i < ASCII_CHARSET_SIZE; i++)
    {
      for (unsigned j = 0; j < ASCII_CHARSET_SIZE; j++)
      {
        if (_mask[p].Satisfy(table[p][i][j].next_state))
          table[p][i][j].probability += UINT16_MAX + 1;
      }
    }
  }
}

std::string CLMarkovPassGen::getPassword(uint64_t index)
{
  uint8_t buffer[256];

  if (index >= _global_stop_index)
    return (string {""});

  // Determine current length
  unsigned length = 1;
  while (index >= _permutations[length])
    length++;

  // Convert global index into local index
  uint64_t local_index = index - _permutations[length - 1];
  uint64_t partial_index;
  uint8_t last_char = 0;

  // Create password
  for (int p = 0; p < length; p++)
  {
    partial_index = local_index % _thresholds[p];
    local_index = local_index / _thresholds[p];

    last_char = _markov_table[p * ASCII_CHARSET_SIZE * _max_threshold
                             + last_char * _max_threshold + partial_index];

    buffer[p] = last_char;
  }

  return (string {(char *) buffer, length});

}
