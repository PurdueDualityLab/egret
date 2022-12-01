/*  TestGenerator.cpp: Generates paths and strings

    Copyright (C) 2016-2018  Eric Larson and Anna Kirk
    elarson@seattleu.edu

    Some code in this file was derived from a RE->NFA converter
    developed by Eli Bendersky.

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

#include "TestGenerator.h"
#include "NFA.h"
#include "Path.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

// TEST STRING GENERATION FUNCTIONS

std::vector<std::string> TestGenerator::gen_test_strings() {
  // get initial strings
  get_initial_strings();

  // TODO: Move to egret.cpp after path processing?
  // debug - print initial strings from basis paths
  if (debug_mode) {
    std::cout << "Initial Test Strings: " << std::endl;
    for (auto &test_string : test_strings) {
      std::cout << test_string << std::endl;
    }
  }

  // gen minimum iteration strings
  gen_min_iter_strings();

  // gen evil strings
  gen_evil_strings();

  // TODO: Create a function that checks for duplicates each time a string is
  // added? create return set with no duplicates
  std::vector<std::string> return_strs;
  for (auto &test_string : test_strings) {
    if (!(std::find(return_strs.begin(), return_strs.end(), test_string) !=
          return_strs.end())) {
      return_strs.insert(return_strs.begin(), test_string);
    }
  }

  // record number of generated strings for stats
  num_gen_strings = return_strs.size();

  return return_strs;
}

void TestGenerator::get_initial_strings() {
  std::vector<Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    test_strings.push_back(path_iter->get_test_string());
  }
}

void TestGenerator::gen_min_iter_strings() {
  if (debug_mode) {
    std::cout << "Minimum Iteration Test Strings: " << std::endl;
  }

  std::vector<Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    std::string min_iter_string = path_iter->gen_min_iter_string();
    test_strings.push_back(min_iter_string);
    if (debug_mode)
      std::cout << min_iter_string << std::endl;
  }
}

void TestGenerator::gen_evil_strings() {
  std::vector<Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    std::vector<std::string> evil_strings =
        path_iter->gen_evil_strings(punct_marks);
    std::vector<std::string>::iterator si;
    for (si = evil_strings.begin(); si != evil_strings.end(); si++) {
      test_strings.push_back(*si);
    }
  }
}

// STAT FUNCTION

void TestGenerator::add_stats(Stats &stats) {
  // TODO: Should paths be included here?
  stats.add("PATHS", "Paths", paths.size());
  stats.add("PATHS", "Strings", num_gen_strings);
}
