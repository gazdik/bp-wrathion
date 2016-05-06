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

#include "Mask.h"

#include <cstring>

#include <stdexcept>

using namespace std;

MaskElement::MaskElement()
{
  _charset_flags.set();
}

MaskElement::MaskElement(const std::string & single_mask)
{
  MaskRangeFunction fn = nullptr;

  // Mask is single character
  if (single_mask.size() == 1)
  {
    _charset_flags[single_mask[0]] = true;
    return;
  }

  // Assign range function
  for (int i = 0; i < NUM_METACHARS; i++)
  {
    if (range_functions[i].ch == single_mask[1])
    {
      fn = range_functions[i].fn;
      break;
    }
  }

  // Set bits according to metacharacter
  if (fn == nullptr)
    throw runtime_error { "Mask: Unknown metacharacter" };

  for (int i = 0; i < ASCII_CHARSET_SIZE; i++)
  {
    _charset_flags[i] = fn(i);
  }
}

const MaskElement& Mask::operator [](std::size_t idx)
{
  if (idx >=_mask_elements.size())
    return _out_of_range;

  return _mask_elements[idx];
}

bool MaskElement::Satisfy(uint8_t character) const
{
  return _charset_flags.test(character);
}

Mask::Mask()
{
}

Mask::Mask(const std::string& mask)
{
  for (unsigned i = 0; i < mask.size(); i++)
  {
    if (mask[i] == '?')
    {
      _mask_elements.push_back(MaskElement {mask.substr(i, 2)});
      i++;
    }
    else
    {
      _mask_elements.push_back(MaskElement {mask.substr(i, 1)});
    }
  }
}

Mask::~Mask()
{
}

std::size_t MaskElement::Count() const
{
  return _charset_flags.count();
}
