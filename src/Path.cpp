/*  Path.cpp: Represents a path through the NFA

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

// TODO: Check proper set of include files
#include "Path.h"
#include "Edge.h"
#include "Util.h"
#include <set>
#include <string>
#include <vector>

// PATH CONSTRUCTION FUNCTIONS

void Path::append(const std::shared_ptr<Edge>& edge, unsigned int state) {
  edges.push_back(edge);
  states.push_back(state);
}

void Path::remove_last() {
  edges.pop_back();
  states.pop_back();
}

void Path::mark_path_visited(bool *visited) {
  for (unsigned int & state : states) {
    visited[state] = true;
  }
}

// PATH PROCESSING FUNCTION

void Path::process_path() {
  // Clear the string to start
  test_string.clear();

  for (unsigned int i = 0; i < edges.size(); i++) {
    // An edge must be processed first before being added, the function returns
    // whether the edge is evil and more tests should be added later.
    bool evil_edge = edges[i]->process_edge(test_string, this);
    if (evil_edge) {
      evil_edges.push_back(i);
    }

    // Add the substring to the initial string.
    test_string.append(edges[i]->get_substring());
  }
}

// CHECKER FUNCTIONS

bool Path::has_leading_caret() {
  for (auto & edge : edges) {
    switch (edge->get_type()) {
    case CARET_EDGE:
      return true;
    case BEGIN_LOOP_EDGE:
    case END_LOOP_EDGE:
    case BACKREFERENCE_EDGE:
    case EPSILON_EDGE:
      break; // Skip over
    default:
      return false;
    }
  }
  return false;
}

bool Path::has_trailing_dollar() {
  for (unsigned int i = edges.size() - 1; i > 0; i--) {
    switch (edges[i]->get_type()) {
    case DOLLAR_EDGE:
      return true;
    case BEGIN_LOOP_EDGE:
    case END_LOOP_EDGE:
    case BACKREFERENCE_EDGE:
    case EPSILON_EDGE:
      break; // Skip over
    default:
      return false;
    }
  }
  return false;
}

bool Path::check_anchor_in_middle() {
  bool seen_non_caret = false;
  bool seen_dollar = false;
  Location seen_non_caret_loc;
  Location seen_dollar_loc;

  for (auto &edge : edges) {
    switch (edge->get_type()) {
    case CARET_EDGE:
      if (seen_non_caret) {
        std::string msg =
            "Generated string has ^ anchor in the middle: " + test_string;
        Alert a("anchor middle", msg, seen_non_caret_loc, edge->get_loc());
        Util::get()->add_alert(a);
        return true;
      }
      break;
    case DOLLAR_EDGE:
      seen_dollar = true;
      seen_dollar_loc = edge->get_loc();
      break;
    case BEGIN_LOOP_EDGE:
    case END_LOOP_EDGE:
    case BACKREFERENCE_EDGE:
    case EPSILON_EDGE:
      break;
    default:
      seen_non_caret = true;
      seen_non_caret_loc = edge->get_loc();
      if (seen_dollar) {
        std::string msg =
            "Generated string has $ anchor in the middle: " + test_string;
        Alert a("anchor middle", msg, seen_dollar_loc, seen_non_caret_loc);
        Util::get()->add_alert(a);
        return true;
      }
    }
  }

  return false;
}

void Path::check_charsets() {
  std::vector<std::string>
      charsets; // keeps track of charsets, looking for duplicates
  std::vector<Location> locs;

  for (auto &edge : edges) {

    if (edge->get_type() == CHAR_SET_EDGE || edge->get_type() == STRING_EDGE) {
      auto charset_ptr = edge->get_charset();
      Location loc = edge->get_loc();

      // check the character set
      charset_ptr->check(this, loc);

      // look for duplicate charsets that only have punctuation
      if (charset_ptr->only_has_punc_and_spaces()) {
        std::string charset_str = charset_ptr->get_charset_as_string();
        bool ignored = false;
        if (charset_str == "+-" || charset_str == "-+") {
          ignored = true;
        }
        if (charset_str.length() > 1 && !ignored) {
          bool found_dup = false;
          for (unsigned int i = 0; i < charsets.size() && !found_dup; i++) {
            if (charset_str == charsets[i]) {
              std::string msg = "Duplicate character set of punctuation marks "
                                "can lead to mismatched punctuation usage";
              char c1 = charset_ptr->get_valid_character();
              char c2 = charset_ptr->get_valid_character(c1);
              Alert a("duplicate punc charset", msg, locs[i], loc);
              a.has_example = true;
              a.example = gen_example_string(locs[i], c1, loc, c2);
              Util::get()->add_alert(a);
              found_dup = true;
            }
          }
          if (!found_dup) {
            // not a duplicate - add to list
            charsets.push_back(charset_str);
            locs.push_back(loc);
          }
        }
      }
    }
  }
}

void Path::check_optional_braces() {
  bool prev_opt_repeat = false;
  bool prev_opt_char = false;
  Location prev_opt_loc;
  char prev_char;

  bool opt_lparen = false;
  bool opt_rparen = false;
  bool opt_lcurly = false;
  bool opt_rcurly = false;
  bool opt_lbrace = false;
  bool opt_rbrace = false;

  Location opt_lparen_loc;
  Location opt_rparen_loc;
  Location opt_lcurly_loc;
  Location opt_rcurly_loc;
  Location opt_lbrace_loc;
  Location opt_rbrace_loc;

  // Traverse path
  for (auto &edge : edges) {
    Location loc = edge->get_loc();
    if (edge->is_opt_repeat_begin()) {
      prev_opt_repeat = true;
      prev_opt_char = false;
    }
    // TODO: This does not capture situations where a group has a single
    // character
    else if (prev_opt_repeat && edge->get_type() == CHARACTER_EDGE) {
      prev_opt_char = true;
      prev_char = edge->get_character();
      prev_opt_repeat = false;
      prev_opt_loc = loc;
    } else if (prev_opt_char && edge->is_opt_repeat_end()) {
      Location l = std::make_pair(prev_opt_loc.first, loc.second);
      prev_opt_char = false;
      prev_opt_repeat = false;
      switch (prev_char) {
      case '(':
        opt_lparen = true;
        opt_lparen_loc = l;
        break;
      case ')':
        opt_rparen = true;
        opt_rparen_loc = l;
        break;
      case '{':
        opt_lcurly = true;
        opt_lcurly_loc = l;
        break;
      case '}':
        opt_rcurly = true;
        opt_rcurly_loc = l;
        break;
      case '[':
        opt_lbrace = true;
        opt_lbrace_loc = l;
        break;
      case ']':
        opt_rbrace = true;
        opt_rbrace_loc = l;
        break;
      default:
        break;
      }
    } else {
      prev_opt_char = false;
      prev_opt_repeat = false;
    }
  }

  // Signal violations
  if (opt_lparen && opt_rparen) {
    std::string msg = "Optional ( and ) found - accepts strings that have one "
                      "but not the other";
    Alert a("optional brace", msg, opt_lparen_loc, opt_rparen_loc);
    a.has_example = true;
    a.example = gen_example_string(opt_lparen_loc, '(', opt_rparen_loc);
    Util::get()->add_alert(a);
  }
  if (opt_lparen && !opt_rparen) {
    std::string msg =
        "Optional ( found - accepts strings that have one but not the other";
    Alert a("optional brace", msg, opt_lparen_loc);
    a.has_example = true;
    a.example = gen_example_string(opt_lparen_loc, '(');
    Util::get()->add_alert(a);
  }
  if (!opt_lparen && opt_rparen) {
    std::string msg =
        "Optional ) found - accepts strings that have one but not the other";
    Alert a("optional brace", msg, opt_rparen_loc);
    a.has_example = true;
    a.example = gen_example_string(opt_rparen_loc, ')');
    Util::get()->add_alert(a);
  }
  if (opt_lcurly && opt_rcurly) {
    std::string msg = "Optional { and } found - accepts strings that have one "
                      "but not the other";
    Alert a("optional brace", msg, opt_lcurly_loc, opt_rcurly_loc);
    a.has_example = true;
    a.example = gen_example_string(opt_lcurly_loc, '{', opt_rcurly_loc);
    Util::get()->add_alert(a);
  }
  if (opt_lcurly && !opt_rcurly) {
    std::string msg =
        "Optional { found - accepts strings that have one but not the other";
    Alert a("optional brace", msg, opt_lcurly_loc);
    a.has_example = true;
    a.example = gen_example_string(opt_lcurly_loc, '{');
    Util::get()->add_alert(a);
  }
  if (!opt_lcurly && opt_rcurly) {
    std::string msg =
        "Optional } found - accepts strings that have one but not the other";
    Alert a("optional brace", msg, opt_rcurly_loc);
    a.has_example = true;
    a.example = gen_example_string(opt_rcurly_loc, '}');
    Util::get()->add_alert(a);
  }
  if (opt_lbrace && opt_rbrace) {
    std::string msg = "Optional [ and ] found - accepts strings that have one "
                      "but not the other";
    Alert a("optional brace", msg, opt_lbrace_loc, opt_rbrace_loc);
    a.has_example = true;
    a.example = gen_example_string(opt_lbrace_loc, '[', opt_rbrace_loc);
    Util::get()->add_alert(a);
  }
  if (opt_lbrace && !opt_rbrace) {
    std::string msg =
        "Optional [ found - accepts strings that have one but not the other";
    Alert a("optional brace", msg, opt_lbrace_loc);
    a.has_example = true;
    a.example = gen_example_string(opt_lbrace_loc, '[');
    Util::get()->add_alert(a);
  }
  if (!opt_lbrace && opt_rbrace) {
    std::string msg =
        "Optional ] found - accepts strings that have one but not the other";
    Alert a("optional brace", msg, opt_rbrace_loc);
    a.has_example = true;
    a.example = gen_example_string(opt_rbrace_loc, ']');
    Util::get()->add_alert(a);
  }
}

void Path::check_wild_punctuation() {
  for (unsigned int i = 0; i < edges.size(); i++) {
    if (edges[i]->is_wild_candidate()) {

      // Get previous edge
      // TODO: Do paths have epsilon edges?  If so, should they be removed?
      int prev_edge = i - 1;
      while (prev_edge >= 0) {
        EdgeType type = edges[prev_edge]->get_type();
        if (type != EPSILON_EDGE && type != BEGIN_LOOP_EDGE &&
            type != END_LOOP_EDGE)
          break;
        prev_edge--;
      }

      // TODO: Could make this code a function since it is repeated
      // Signal violation if previous edge is single character that is a
      // punctuation mark
      if (prev_edge != -1 && edges[prev_edge]->get_type() == CHARACTER_EDGE) {
        char c = edges[prev_edge]->get_character();
        Location prev_loc = edges[prev_edge]->get_loc();
        if (ispunct(c) && edges[i]->is_valid_character(c)) {
          Location loc = edges[i]->get_loc();
          std::string fix = edges[i]->fix_wild_punctuation(c);
          std::string msg =
              "Wildcard may wish to exclude adjacent punctuation mark " +
              std::string(1, c);
          Alert a("wild punctuation", msg, fix, loc, prev_loc);
          a.has_example = true;
          a.example = gen_example_string(loc, c);
          Util::get()->add_alert(a);
        }
      }

      // Get next edge
      unsigned int next_edge = i + 1;
      while (next_edge < edges.size()) {
        EdgeType type = edges[next_edge]->get_type();
        if (type != EPSILON_EDGE && type != BEGIN_LOOP_EDGE &&
            type != END_LOOP_EDGE)
          break;
        next_edge++;
      }

      // Signal violation if next edge is single character that is a punctuation
      // mark
      if (next_edge != edges.size() &&
          edges[next_edge]->get_type() == CHARACTER_EDGE) {
        char c = edges[next_edge]->get_character();
        Location next_loc = edges[next_edge]->get_loc();
        if (ispunct(c) && edges[i]->is_valid_character(c)) {
          Location loc = edges[i]->get_loc();
          std::string fix = edges[i]->fix_wild_punctuation(c);
          std::string msg =
              "Wildcard may wish to exclude adjacent punctuation mark " +
              std::string(1, c);
          Alert a("wild punctuation", msg, fix, loc, next_loc);
          a.has_example = true;
          a.example = gen_example_string(loc, c);
          Util::get()->add_alert(a);
        }
      }
    }
  }
}

void Path::check_repeat_punctuation() {
  bool prev_repeat = false;
  bool prev_candidate = false;
  char prev_char;
  Location prev_loc;

  for (auto &edge : edges) {
    Location curr_loc = edge->get_loc();
    if (edge->is_str_repeat_punc_candidate()) {
      char c = edge->get_repeat_punc_char();

      std::string repeat_str = std::string(1, c);
      int limit = 3;
      int lower_limit = edge->get_repeat_lower_limit();
      int upper_limit = edge->get_repeat_upper_limit();

      if (lower_limit > 3) {
        limit = lower_limit;
      } else if (upper_limit == 2) {
        limit = upper_limit;
      }
      for (int i = 1; i < limit; i++) {
        repeat_str += c;
      }

      if (lower_limit != upper_limit) {
        std::string msg =
            "Punctuation mark may be repeated two or more times: " +
            std::string(1, c);
        Alert a("repeat punctuation", msg, curr_loc);
        a.has_example = true;
        a.example = gen_example_string(curr_loc, repeat_str);
        Util::get()->add_alert(a);
      }
    } else if (edge->is_repeat_begin()) {
      prev_repeat = true;
      prev_candidate = false;
    }
    // TODO: This does not capture situations where a group has a single
    // character
    else if (prev_repeat && edge->is_repeat_punc_candidate()) {
      prev_char = edge->get_repeat_punc_char();
      prev_repeat = false;
      prev_candidate = true;
      prev_loc = curr_loc;
    } else if (prev_candidate && edge->is_repeat_end()) {
      Location loc = std::make_pair(prev_loc.first, curr_loc.second);
      prev_repeat = false;
      prev_candidate = false;

      std::string repeat_str = std::string(1, prev_char);
      int limit = 3;
      int lower_limit = edge->get_repeat_lower_limit();
      int upper_limit = edge->get_repeat_upper_limit();

      if (lower_limit > 3) {
        limit = lower_limit;
      } else if (upper_limit == 2) {
        limit = upper_limit;
      }
      for (int i = 1; i < limit; i++) {
        repeat_str += prev_char;
      }

      if (lower_limit != upper_limit) {
        std::string msg =
            "Punctuation mark may be repeated two or more times: " +
            std::string(1, prev_char);
        Alert a("repeat punctuation", msg, prev_loc, curr_loc);
        a.has_example = true;
        a.example = gen_example_string(loc, repeat_str);
        Util::get()->add_alert(a);
      }
    } else {
      prev_repeat = false;
      prev_candidate = false;
    }
  }
}

void Path::check_digit_too_optional() {
  bool prev_repeat = false;
  bool prev_candidate = false;
  Location prev_loc;

  for (auto & edge : edges) {
    Location curr_loc = edge->get_loc();
    if (edge->is_zero_repeat_begin()) {
      prev_repeat = true;
      prev_candidate = false;
    } else if (prev_repeat && edge->is_digit_too_optional_candidate()) {
      prev_repeat = false;
      prev_candidate = true;
      prev_loc = curr_loc;
    } else if (prev_candidate && edge->is_zero_repeat_end()) {
      prev_repeat = false;
      prev_candidate = false;
      std::string example = gen_min_iter_string();

      bool found_digit = false;
      for (char c : example) {
        if (c >= '0' && c <= '9')
          found_digit = true;
      }

      if (!found_digit) {
        Location loc = std::make_pair(prev_loc.first, curr_loc.second);
        std::string msg = "Digit range allows for zero digits casuing a string "
                          "with no digits to be accepted";
        Alert a("digit too optional", msg, loc);
        a.has_example = true;
        a.example = example;
        Util::get()->add_alert(a);
      }
    } else {
      prev_repeat = false;
      prev_candidate = false;
    }
  }
}

// TEST STRING GENERATION FUNCTIONS

std::string Path::gen_example_string(Location loc, char c) {
  std::string example;
  for (auto &edge : edges) {
    edge->process_edge(example, this);

    Location edge_loc = edge->get_loc();
    if (edge_loc.first == loc.first) {
      example += c;
    } else {
      example += edge->get_substring();
    }
  }

  return example;
}

std::string Path::gen_example_string(Location loc, char c, char except) {
  std::string example;
  for (auto &edge : edges) {
    edge->process_edge(example, this);

    Location edge_loc = edge->get_loc();
    if (edge_loc.first == loc.first) {
      example += c;
    } else {
      std::string sub = edge->get_substring();
      std::string except_str = std::string(1, except);
      if (sub == except_str) {
        if (edge->get_type() == CHAR_SET_EDGE) {
          char loc_c = edge->get_charset()->get_valid_character(except);
          example += loc_c;
        } else {
          example += edge->get_substring();
        }
      } else {
        example += edge->get_substring();
      }
    }
  }

  return example;
}

std::string Path::gen_example_string(Location loc, char c, Location omit) {
  std::string example;
  for (auto &edge : edges) {
    edge->process_edge(example, this);

    Location edge_loc = edge->get_loc();
    if (edge_loc.first == loc.first) {
      example += c;
    } else if (edge_loc.first == omit.first) {
      continue;
    } else {
      example += edge->get_substring();
    }
  }

  return example;
}

std::string Path::gen_example_string(Location loc1, char c1, Location loc2,
                                     char c2) {
  std::string example;
  for (auto &edge : edges) {
    edge->process_edge(example, this);

    Location edge_loc = edge->get_loc();
    if (edge_loc.first == loc1.first) {
      example += c1;
    } else if (edge_loc.first == loc2.first) {
      example += c2;
    } else {
      example += edge->get_substring();
    }
  }

  return example;
}

std::string Path::gen_example_string(Location loc, const std::string &replace) {
  std::string example;
  bool in_replace = false;
  for (auto &edge : edges) {
    edge->process_edge(example, this);

    Location edge_loc = edge->get_loc();
    if (edge_loc.first == loc.first) {
      example += replace;
      in_replace = (edge_loc.second != loc.second);
    } else if (edge_loc.second == loc.second) {
      in_replace = false;
    } else if (!in_replace) {
      example += edge->get_substring();
    }
  }

  return example;
}

std::string Path::gen_backref_string(Location loc) {
  // Assumes edges have been processed (called during test generation)
  std::string backref;
  for (auto &edge : edges) {
    Location edge_loc = edge->get_loc();
    if (edge_loc.first > loc.first && edge_loc.first < loc.second) {
      backref += edge->get_substring();
    }
  }
  return backref;
}

std::string Path::gen_min_iter_string() {
  std::string min_iter_string;
  for (auto &edge : edges) {
    edge->gen_min_iter_string(min_iter_string);
  }
  return min_iter_string;
}

std::vector<std::string>
Path::gen_evil_strings(const std::set<char> &punct_marks) {
  std::vector<std::string> evil_strings;

  // add strings for interesting edges (char sets, strings, and loops)
  for (int index : evil_edges) {
    std::vector<std::string> new_strings =
        edges[index]->gen_evil_strings(test_string, punct_marks);
    std::vector<std::string>::iterator tsi;
    for (tsi = new_strings.begin(); tsi != new_strings.end(); tsi++) {
      evil_strings.push_back(*tsi);
    }
  }
  return evil_strings;
}

// PRINT FUNCTION

void Path::print() {
  for (auto &edge : edges)
    edge->print();
}
