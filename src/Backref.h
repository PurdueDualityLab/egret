/*  Backref.h: represents a backreference

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

#ifndef BACKREF_H
#define BACKREF_H

#include <string>
#include <utility>
#include <vector>
#include "Util.h"

class Backref {

public:

  Backref(std::string name, int number, Location l) {
    group_name = std::move(name);
    group_number = number;
    group_loc = l;
  }

  // setters
  void set_curr_prefix(std::string s) { curr_prefix = std::move(s); }
  void set_curr_substring(std::string s) { curr_substring = std::move(s); }
  void set_prefix_from_curr() { prefix = curr_prefix; }
  void set_substring_from_curr() { substring = curr_substring; }

  // getters
  Location get_group_loc() { return group_loc; }
    std::string get_substring() { return substring; }

  // generate minimum iteration string
  void gen_min_iter_string(std::string &min_iter_string);

  // generate evil strings
  std::vector <std::string> gen_evil_strings(std::string test_string);

  // print the regex loop
  void print();

private:
    std::string group_name;            // name of group (blank if using numbered backreference)
    int group_number;             // number of group
    Location group_loc;           // location of group

    std::string prefix;       	        // prefix of test string before the backreference
    std::string substring;    	        // substring corresponding to backreference

    std::string curr_prefix;           // current path string up to visiting this node
    std::string curr_substring;        // current substring corresponding to this backreference
};

#endif // BACKREF_H

