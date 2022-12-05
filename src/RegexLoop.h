/*  RegexLoop.h: represents a regex repeat quantifier

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

#ifndef REGEX_LOOP_H
#define REGEX_LOOP_H

#include <string>
#include <utility>
#include <vector>

class RegexLoop {

public:
  RegexLoop(int lower, int upper) {
    repeat_lower = lower;
    repeat_upper = upper;
  }

  // setters
  void set_prefix(std::string p) { prefix = std::move(p); }
  void set_substring_from_curr() { substring = curr_substring; }
  void set_curr_prefix(std::string p) { curr_prefix = std::move(p); }
  void set_curr_substring(const std::string &test_string);

  // getters
  int get_repeat_lower() const { return repeat_lower; }
  int get_repeat_upper() const { return repeat_upper; }
  std::string get_substring();

  // property functions - used by checker
  bool is_opt_repeat() const;

  // generate minimum iteration string
  void gen_min_iter_string(std::string &min_iter_string);

  // generate evil strings
  std::vector<std::string> gen_evil_strings(const std::string& test_string);

  // print the regex loop
  void print() const;

private:
  int repeat_lower; // lower bound for repeat quantifiers
  int repeat_upper; // upper bound for repeat quantifiers (-1 if no bound)

  // TODO: Rename these to something like evil prefix
  std::string prefix;    // prefix of test string before the loop
  std::string substring; // substring corresponding to loop

  std::string curr_prefix;    // current path string up to visiting this node
  std::string curr_substring; // current substring corresponding to this string
};

#endif // REGEX_LOOP_H
