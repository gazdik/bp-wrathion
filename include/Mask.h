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

#ifndef MASK_H_
#define MASK_H_

#include "PassGen.h"

#include <cstdint>

#include <string>
#include <bitset>
#include <vector>

typedef bool (*MaskRangeFunction)(uint8_t);
struct MaskRange
{
  char ch;
  MaskRangeFunction fn;
};

const unsigned NUM_METACHARS = 7;
static const MaskRange range_functions[NUM_METACHARS] =
{
    {'l', [] (uint8_t ch) { return ch >= 'a' && ch <= 'z'; } },
    {'u', [] (uint8_t ch) { return ch >= 'A' && ch <= 'Z'; } },
    {'c', [] (uint8_t ch) { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch < 'Z'); } },
    {'d', [] (uint8_t ch) { return ch >= '0' && ch <= '9'; } },
    {'a', [] (uint8_t ch) { return ch >= ' ' && ch <= '~'; } },
    {'s', [] (uint8_t ch) { return (ch >= ' ' && ch <= '/') || (ch >= ':' && ch <= '@') || (ch >= '[' && ch <= '`') || (ch >= '{' && ch <= '~'); } },
    {'?', [] (uint8_t ch) { return ch == '?'; } }
};

class MaskElement
{
public:
  MaskElement();
  MaskElement(const std::string & single_mask);

  /**
   * Test if given character satisfy the mask
   */
  bool Satisfy(uint8_t character) const;

  /**
   * Returns number of characters that satisfy mask
   */
  std::size_t Count() const;
private:
  std::bitset<ASCII_CHARSET_SIZE> _charset_flags;
};

class Mask
{
public:
  Mask();
  Mask(const std::string &mask);
  ~Mask();

  const MaskElement & operator[](std::size_t idx);
private:
  std::vector<MaskElement> _mask_elements;
  MaskElement _out_of_range;
};

#endif /* MASK_H_ */
