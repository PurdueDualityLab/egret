/*  Util.cpp: Singleton utility class

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

#include "Util.h"
#include <sstream>
#include <string>
#include <utility>

// global static pointer for singleton class
std::shared_ptr<Util> Util::inst;

std::shared_ptr<Util> Util::get() {
  if (!inst)
    inst = std::shared_ptr<Util>(new Util);
  return inst;
}

void Util::init(std::string r, bool c, bool w, std::string s) {
  regex = std::move(r);
  check_mode = c;
  web_mode = w;
  base_substring = std::move(s);
  alerts.clear();
  prev_alerts.clear();
}

void Util::add_alert(const Alert& alert) {
  // Create type, location pair
  std::pair<std::string, int> alert_pair =
      make_pair(alert.type, alert.loc1.first);

  // Line break
  std::string lb = web_mode ? "<br>" : "\n";
  std::string start = web_mode ? "<mark>" : "\033[33;44;1m";
  std::string end = web_mode ? "</mark>" : "\033[0m";

  if (prev_alerts.find(alert_pair) == prev_alerts.end()) {
    // New error - add to list of previous alerts
    prev_alerts.insert(alert_pair);
  } else {
    // Duplicate - do not add the error again
    return;
  }

  // Ignore warnings in check mode (warnings only relevant in test generation
  // mode)
  if (alert.warning && check_mode)
    return;

  // Produce alert message
  std::stringstream s;
  if (alert.warning)
    s << "WARNING (";
  else
    s << "VIOLATION (";
  s << alert.type << "): " << alert.message << lb;

  if (alert.loc1.first != -1) {
    s << "...Regex: ";

    for (int i = 0; i < (int)regex.size(); i++) {
      if (i == alert.loc1.first || i == alert.loc2.first) {
        s << start;
      }
      s << regex[i];
      if (i == alert.loc1.second || i == alert.loc2.second) {
        s << end;
      }
    }
    s << lb;
  }

  if (alert.has_suggest) {
    s << "...Suggested fix: " << alert.suggest << lb;
  }

  if (alert.has_example) {
    s << "...Example accepted string: " << alert.example << lb;
  }
  alerts.push_back(s.str());
}
