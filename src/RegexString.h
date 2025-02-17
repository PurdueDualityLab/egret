/*  RegexString.h: represents a regex string

    Copyright (C) 2016-2018  Eric Larson and Anna Kirk
    elarson@seattleu.edu

    This file is part of EGRET.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef REGEX_STRING_H
#define REGEX_STRING_H

#include "CharSet.h"
#include "Util.h"
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

class RegexString {

public:
  RegexString(std::shared_ptr<CharSet> c, int lower, int upper) {
    char_set = std::move(c);
    repeat_lower = lower;
    repeat_upper = upper;
  }

  // setters
  void set_prefix(std::string p) { prefix = std::move(p); }
  void set_substring(std::string s) { substring = std::move(s); }

  // getters
  std::string get_substring() { return substring; }
  int get_repeat_lower() const { return repeat_lower; }
  int get_repeat_upper() const { return repeat_upper; }
  std::shared_ptr<CharSet> get_charset() { return char_set; }

  // property function - used by checker
  // TODO: These functions could be refactored
  bool is_wild_candidate() {
    return char_set->is_wildcard() || char_set->is_complement();
  }
  bool is_valid_character(char c) {
    return char_set->is_wildcard() || char_set->is_valid_character(c);
  }

  // repeat punctuation check functions
  bool is_repeat_punc_candidate() {
    return char_set->is_repeat_punc_candidate();
  }
  char get_repeat_punc_char() { return char_set->get_repeat_punc_char(); }

  // generate minimum iterations string
  void gen_min_iter_string(std::string &min_iter_string);

  // generate evil strings
  std::vector<std::string> gen_evil_strings(const std::string &test_string,
                                            const std::set<char> &punct_marks);

  // print the regex string
  void print();

private:
  std::shared_ptr<CharSet> char_set; // corresponding character set
  int repeat_lower;  // lower bound for string
  int repeat_upper;  // upper bound for string

  std::string prefix;    // prefix of test string before the loop
  std::string substring; // substring corresponding to this string
};

#endif // REGEX_STRING_H
