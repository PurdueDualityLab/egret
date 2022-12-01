/*  Stats.cpp: EGRET stats

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

#include "Stats.h"
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

void Stats::add(std::string tag, std::string name, int value) {
  Stat stat = {std::move(tag), std::move(name), value};
  statList.push_back(stat);
}

void Stats::print() {
  const int WIDTH = 30;

  std::string prev_tag;
  std::vector<Stat>::iterator it;
  for (it = statList.begin(); it != statList.end(); it++) {

    // print divider line between different tags
    if (it->tag != prev_tag && !prev_tag.empty()) {
      for (int i = 0; i < WIDTH + 8; i++)
        std::cout << "-";
      std::cout << std::endl;
    }

    std::cout << std::left << std::setw(WIDTH) << it->name << "| " << it->value
              << std::endl;
    prev_tag = it->tag;
  }
}
