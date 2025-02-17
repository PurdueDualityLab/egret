/*  egret.cpp: entry point into EGRET engine

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

#include "Checker.h"
#include "NFA.h"
#include "ParseTree.h"
#include "Path.h"
#include "Scanner.h"
#include "Stats.h"
#include "TestGenerator.h"
#include "Util.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<std::string>
run_engine(const std::string& regex, const std::string& base_substring,
           bool check_mode = false, bool web_mode = false,
           bool debug_mode = false, bool stat_mode = false) {
  Stats stats;
  std::vector<std::string> test_strings;

  try {

    // check and convert base substring
    if (base_substring.length() < 2) {
      throw EgretException("ERROR (bad arguments): Base substring must have at "
                           "least two letters");
    }

    for (char i : base_substring) {
      if (!isalpha(i)) {
        throw EgretException(
            "ERROR (bad arguments): Base substring can only contain letters");
      }
    }

    // set global options
    Util::get()->init(regex, check_mode, web_mode, base_substring);

    // start debug mode
    if (debug_mode)
      std::cout << "RegEx: " << regex << std::endl;

    // initialize scanner with regex
    Scanner scanner;
    scanner.init(regex);
    if (debug_mode)
      scanner.print();
    if (stat_mode)
      scanner.add_stats(stats);

    // build parse tree
    ParseTree tree;
    tree.build(scanner);
    if (debug_mode)
      tree.print();
    if (stat_mode)
      tree.add_stats(stats);

    // store stuff out of tree before it gets moved
    auto punct_marks = tree.get_punct_marks();

    // build NFA
    NFA nfa;
    nfa.build(tree);
    if (debug_mode)
      nfa.print();
    if (stat_mode)
      nfa.add_stats(stats);

    // traverse NFA basis paths and process them
    std::vector<Path> paths = nfa.find_basis_paths();
    for (auto &path : paths) {
      path.process_path();
    }

    // run checker
    if (check_mode) {
      Checker checker(paths, scanner.get_tokens());
      checker.check();
    }

    // generate tests
    if (!check_mode) {
      TestGenerator gen(paths, punct_marks, debug_mode);
      test_strings = gen.gen_test_strings();
      if (stat_mode)
        gen.add_stats(stats);
    }

    // print stats
    if (stat_mode)
      stats.print();
  } catch (EgretException const &e) {
    throw std::runtime_error(e.get_error());
  }

  // Add alerts to front of list.
  std::vector<std::string> alerts = Util::get()->get_alerts();
  if (check_mode) {
    if (alerts.empty()) {
      alerts.insert(alerts.begin(), "No violations detected.");
    }
    return alerts;
  }
  test_strings.insert(test_strings.begin(), "BEGIN");
  test_strings.insert(test_strings.begin(), alerts.begin(), alerts.end());

  return test_strings;
}
