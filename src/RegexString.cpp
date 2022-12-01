/*  RegexString.cpp: represents a regex string

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

#include "RegexString.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <string>

void RegexString::gen_min_iter_string(std::string &min_iter_string) {
  if (repeat_lower != 0) {
    min_iter_string.append(get_substring());
  }
}

std::vector<std::string>
RegexString::gen_evil_strings(const std::string &test_string,
                              const std::set<char> &punct_marks) {
  std::vector<std::string> evil_substrings; // set of evil substrings

  // create suffix: substring after the loop
  int start = prefix.size() + substring.size();
  std::string suffix = test_string.substr(start);

  // insert one letter strings
  evil_substrings.push_back("");
  evil_substrings.push_back("_");
  evil_substrings.push_back("6");
  evil_substrings.push_back(" ");

  // insert string with just first character of substring
  evil_substrings.push_back(substring.substr(0, 1));

  // split test string into two halves
  unsigned int half = substring.size() / 2;
  std::string before = substring.substr(0, half);
  std::string after = substring.substr(half);

  // insert strings with added digit, space, and underscore
  evil_substrings.push_back(before + "4" + after);
  evil_substrings.push_back(before + " " + after);
  evil_substrings.push_back(before + "_" + after);

  // insert all uppercase, all lowercase, and mixed case where
  // the first char is lowercase and the second char is uppercase
  std::string all_upper = substring;
  std::string all_lower = all_upper;
  std::string mixed = all_upper;

  for (unsigned int i = 0; i < substring.size(); i++) {
    all_upper[i] = toupper(all_upper[i], std::locale());
    all_lower[i] = tolower(all_lower[i], std::locale());
    if (i == 0) {
      mixed[i] = tolower(mixed[i], std::locale());
    } else if (i == 1) {
      mixed[i] = toupper(mixed[i], std::locale());
    }
  }
  evil_substrings.push_back(all_upper);
  evil_substrings.push_back(all_lower);
  evil_substrings.push_back(mixed);

  // insert strings for each punctation mark
  if (char_set->allows_punctuation()) {
    for (char punct_mark : punct_marks) {
      // TODO: This string constructor the most logical?
      evil_substrings.emplace_back(1, punct_mark);
    }
  }

  // generate the new full strings
  std::vector<std::string> evil_strings;
  std::vector<std::string>::iterator tsi;
  for (tsi = evil_substrings.begin(); tsi != evil_substrings.end(); tsi++) {
    std::string new_string = prefix + *tsi + suffix;
    evil_strings.push_back(new_string);
  }

  return evil_strings;
}

void RegexString::print() {
  char_set->print();

  if (repeat_lower == 0 && repeat_upper == -1)
    std::cout << "*";
  else if (repeat_lower == 1 && repeat_upper == -1)
    std::cout << "+";
  else if (repeat_lower == 0 && repeat_upper == 1)
    std::cout << "?";
  else if (repeat_upper == -1)
    std::cout << "{" << repeat_lower << ",}";
  else if (repeat_lower == repeat_upper)
    std::cout << "{" << repeat_lower << "}";
  else
    std::cout << "{" << repeat_lower << "," << repeat_upper << "}";
}
