/*  ParseTree.cpp: recursive descent parser

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

#include "ParseTree.h"
#include "Backref.h"
#include "CharSet.h"
#include "Scanner.h"
#include "Stats.h"
#include "Util.h"
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

//=============================================================
// RD Parser
//=============================================================

void ParseTree::build(Scanner &_scanner) {
  group_count = 1;

  scanner = _scanner;
  root = expr();

  if (scanner.get_type() != ERR) {
    std::stringstream s;
    s << "ERROR (parse error): expected end of regex but received "
      << scanner.get_type_str();
    throw EgretException(s.str());
  }
  // count_groups();
}

// expr ::= concat '|' expr
//	|   concat '|'
//	|   '|' expr
//	|   '|'
//      |   concat
//
std::unique_ptr<ParseNode> ParseTree::expr() {
  std::unique_ptr<ParseNode> left, right;

  // check for alternation without a "left"
  if (scanner.get_type() != ALTERNATION) {
    left = concat();
  }

  // check for lack of alternation
  if (scanner.get_type() != ALTERNATION) {
    return left;
  }

  // advance past alternation token
  Location loc = scanner.get_loc();
  scanner.advance();

  // check for lacking right
  if (!(scanner.get_type() == RIGHT_PAREN || scanner.get_type() == ERR)) {
    right = expr();
  }

  // check for empty alternation clauses
  // both empty: abort with an error
  if (left && right) {
    throw EgretException(
        "ERROR (pointless alternation): both clauses are empty");
  }
  // left empty: return right?
  else if (!left) {
    // ParseNode *expr_node = new ParseNode(REPEAT_NODE, loc, right, 0, 1);
    auto expr_node = std::make_unique<ParseNode>(REPEAT_NODE, loc, std::move(right), 0, 1);
    return expr_node;
  }
  // right empty: return left?
  else if (!right) {
    // ParseNode *expr_node = new ParseNode(REPEAT_NODE, loc, left, 0, 1);
    return std::make_unique<ParseNode>(REPEAT_NODE, loc, std::move(left), 0, 1);
  }

  // otherwise return left | right
  // ParseNode *expr_node = new ParseNode(ALTERNATION_NODE, loc, left, right);
  return std::make_unique<ParseNode>(ALTERNATION_NODE, loc, std::move(left), std::move(right));
}

// concat ::= rep concat
//        |   rep
//
std::unique_ptr<ParseNode> ParseTree::concat() {
  // always a repetition node to the left
  std::unique_ptr<ParseNode> left = rep();

  // check for concatenation
  if (scanner.is_concat()) {
    std::unique_ptr<ParseNode> right = concat();
    int left_loc = left->loc.second;
    Location loc = std::make_pair(left_loc, left_loc + 1);
    // ParseNode *concat_node = new ParseNode(CONCAT_NODE, loc, left, right);
    // return concat_node;
    return std::make_unique<ParseNode>(CONCAT_NODE, loc, std::move(left), std::move(right));
  } else {
    return left;
  }
}

// rep  ::= atom '*'
//      |   atom '?'
//      |   atom '+'
//      |   atom '{n,m}'
//      |   atom '{n,}'
//      |   atom
//
std::unique_ptr<ParseNode> ParseTree::rep() {
  // first is always atom node
  std::unique_ptr<ParseNode> atom_node = atom();
  Location loc = scanner.get_loc();

  // then check for repetition character
  if (scanner.get_type() == STAR) {
    scanner.advance();
    // ParseNode *rep_node = new ParseNode(REPEAT_NODE, loc, atom_node, 0, -1);
    // return rep_node;
    return std::make_unique<ParseNode>(REPEAT_NODE, loc, std::move(atom_node), 0, -1);
  } else if (scanner.get_type() == PLUS) {
    scanner.advance();
    // ParseNode *rep_node = new ParseNode(REPEAT_NODE, loc, atom_node, 1, -1);
    // return rep_node;
    return std::make_unique<ParseNode>(REPEAT_NODE, loc, std::move(atom_node), 1, -1);
  } else if (scanner.get_type() == QUESTION) {
    scanner.advance();
    // ParseNode *rep_node = new ParseNode(REPEAT_NODE, loc, atom_node, 0, 1);
    // return rep_node;
    return std::make_unique<ParseNode>(REPEAT_NODE, loc, std::move(atom_node), 0, 1);
  } else if (scanner.get_type() == REPEAT) {
    int lower = scanner.get_repeat_lower();
    int upper = scanner.get_repeat_upper();
    scanner.advance();
    // ParseNode *rep_node =
    //     new ParseNode(REPEAT_NODE, loc, atom_node, lower, upper);
    // return rep_node;
    return std::make_unique<ParseNode>(REPEAT_NODE, loc, std::move(atom_node), lower, upper);
  } else {
    return atom_node;
  }
}

// atom	::= group
// 	|   character
//	|   char_class
// 	|   char_set
//
std::unique_ptr<ParseNode> ParseTree::atom() {
  std::unique_ptr<ParseNode> atom_node;

  // check for group
  if (scanner.get_type() == LEFT_PAREN) {
    atom_node = group();
  }

  // check for character set
  else if (scanner.get_type() == LEFT_BRACKET) {
    atom_node = char_set();
  }

  // check for character class
  else if (scanner.get_type() == CHAR_CLASS) {
    atom_node = char_class();
  }

  // otherwise atom node is character
  else {
    atom_node = character();
  }

  return atom_node;
}

// group ::= '(' expr ')'
//       | '(' NO_GROUP_EXT expr ')'
//       | '(' NAMED_GROUP_EXT expr ')'
//       | '(' IGNORED_EXT expr ')'
//       | '(' IGNORED_EXT ')'
//
std::unique_ptr<ParseNode> ParseTree::group() {
  bool ignored_group = false;
  bool normal_group = true;
  std::string name;
  int start_loc = scanner.get_loc().second;

  if (scanner.get_type() != LEFT_PAREN) {
    std::stringstream s;
    s << "ERROR (parse error): expected '(' but received "
      << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  // Determine if it a special use of parentheses
  if (scanner.get_type() == NO_GROUP_EXT) {
    normal_group = false;
    scanner.advance();
  }
  if (scanner.get_type() == NAMED_GROUP_EXT) {
    name = scanner.get_group_name();
    scanner.advance();
  }
  if (scanner.get_type() == IGNORED_EXT) {
    normal_group = false;
    ignored_group = true;
    scanner.advance();
  }

  // Assign the group number now before advancing scanner
  int group_num;
  if (normal_group) {
    group_num = group_count;
    group_count++;
  } else {
    group_num = -1;
  }

  // Get the group expression
  std::unique_ptr<ParseNode> left;
  if (!ignored_group || scanner.get_type() != RIGHT_PAREN) {
    left = expr();
  }

  // Create the group node
  std::unique_ptr<ParseNode> group_node;
  int end_loc = scanner.get_loc().first;
  Location loc = std::make_pair(start_loc, end_loc);
  if (ignored_group) {
    // group_node = new ParseNode(IGNORED_NODE, loc, NULL, NULL);
    group_node = std::make_unique<ParseNode>(IGNORED_NODE, loc, std::unique_ptr<ParseNode>(), std::unique_ptr<ParseNode>());
  } else {
    // group_node = new ParseNode(GROUP_NODE, loc, name, std::move(left), nullptr);
    group_node = std::make_unique<ParseNode>(GROUP_NODE, loc, name, std::move(left), std::unique_ptr<ParseNode>());
  }

  // Store group information
  if (normal_group) {
    group_locs[group_num] = loc;
    if (!name.empty()) {
      named_group_locs[name] = loc;
    }
  }

  if (scanner.get_type() != RIGHT_PAREN) {
    std::stringstream s;
    s << "ERROR (parse error): expected ')' but received "
      << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  return group_node;
}

// character ::= CHARACTER
// 	     |   '^'
// 	     |   '$'
//	     |	 '-'
//	     |   WORD_BOUNDARY
//
std::unique_ptr<ParseNode> ParseTree::character() {
  std::unique_ptr<ParseNode> character_node;
  Location loc = scanner.get_loc();
  TokenType type = scanner.get_type();

  if (type == CHARACTER) {
    char c = scanner.get_character();
    scanner.advance();
    // character_node = new ParseNode(CHARACTER_NODE, loc, c);
    character_node = std::make_unique<ParseNode>(CHARACTER_NODE, loc, c);
    if (ispunct(c)) {
      if (punct_marks.find(c) == punct_marks.end()) {
        punct_marks.insert(c);
      }
    }
  } else if (type == CARET) {
    scanner.advance();
    // return new ParseNode(CARET_NODE, loc, NULL, NULL);
    return std::make_unique<ParseNode>(CARET_NODE, loc);
  } else if (type == DOLLAR) {
    scanner.advance();
    // return new ParseNode(DOLLAR_NODE, loc, NULL, NULL);
    return std::make_unique<ParseNode>(DOLLAR_NODE, loc);
  } else if (type == HYPHEN) {
    scanner.advance();
    // character_node = new ParseNode(CHARACTER_NODE, loc, '-');
    character_node = std::make_unique<ParseNode>(CHARACTER_NODE, loc, '-');
    if (punct_marks.find('-') == punct_marks.end()) {
      punct_marks.insert('-');
    }
  } else if (type == WORD_BOUNDARY) {
    scanner.advance();
    // return new ParseNode(IGNORED_NODE, loc, nullptr, nullptr);
    return std::make_unique<ParseNode>(IGNORED_NODE, loc);
  } else if (type == BACKREFERENCE) {
    int group_num = scanner.get_group_num();
    std::string group_name = scanner.get_group_name();
    Location group_loc;
    if (!group_name.empty()) {
      group_loc = named_group_locs[group_name];
    } else {
      group_loc = group_locs[group_num];
    }

    Backref *backref = new Backref(group_name, group_num, group_loc);
    // character_node = new ParseNode(BACKREFERENCE_NODE, loc, backref);
    character_node = std::make_unique<ParseNode>(BACKREFERENCE_NODE, loc, backref);
    scanner.advance();
  } else {
    std::stringstream s;
    s << "ERROR (parse error): expected character type but received "
      << scanner.get_type_str();
    throw EgretException(s.str());
  }

  return character_node;
}

// char_class ::= CHAR_CLASS
//
std::unique_ptr<ParseNode> ParseTree::char_class() {
  Location loc = scanner.get_loc();
  char c = scanner.get_character();
  scanner.advance();

  // TODO fix this
  // CharSet *char_set = new CharSet();
  auto char_set = std::make_unique<CharSet>();

  CharSetItem char_set_item {};
  char_set_item.type = CHAR_CLASS_ITEM;
  char_set_item.character = c;
  char_set->add_item(char_set_item);

  // ParseNode *char_set_node = new ParseNode(CHAR_SET_NODE, loc, char_set);
  // return char_set_node;
  return std::make_unique<ParseNode>(CHAR_SET_NODE, loc, std::move(char_set));
}

// char_set ::= '[' char_list ']'
// 	    |   '[' '^' char_list ']'
//
std::unique_ptr<ParseNode> ParseTree::char_set() {
  std::unique_ptr<ParseNode> char_set_node;
  bool is_complement = false;
  int start_loc = scanner.get_loc().second;

  if (scanner.get_type() != LEFT_BRACKET) {
    std::stringstream s;
    s << "ERROR (parse error): expected '[' but received "
      << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  if (scanner.get_type() == CARET) {
    is_complement = true;
    scanner.advance();
  }

  char_set_node = char_list(start_loc);
  if (is_complement)
    char_set_node->char_set->set_complement(true);
  if (char_set_node->char_set->is_single_char() && !is_complement) {
    char c = char_set_node->char_set->get_valid_character();
    // delete char_set_node;
    char_set_node.reset(nullptr);
    int end_loc = scanner.get_loc().first;
    Location loc = std::make_pair(start_loc, end_loc);
    // char_set_node = new ParseNode(CHARACTER_NODE, loc, c);
    char_set_node = std::make_unique<ParseNode>(CHARACTER_NODE, loc, c);
  }

  if (scanner.get_type() != RIGHT_BRACKET) {
    std::stringstream s;
    s << "ERROR (parse error): expected ']' but received "
      << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  return char_set_node;
}

// char_list ::= list_item charlist
// 	     |   list_item
//
std::unique_ptr<ParseNode> ParseTree::char_list(int start_loc) {
  CharSetItem char_set_item = list_item();
  std::unique_ptr<ParseNode> char_set_node;

  // Check for end of list
  if (scanner.get_type() == RIGHT_BRACKET) {
    int end_loc = scanner.get_loc().first;
    Location loc = std::make_pair(start_loc, end_loc);
    // char_set_node = new ParseNode(CHAR_SET_NODE, loc, new CharSet());
    char_set_node = std::make_unique<ParseNode>(CHAR_SET_NODE, loc, std::make_unique<CharSet>());
  } else {
    char_set_node = char_list(start_loc);
  }

  char_set_node->char_set->add_item(char_set_item);
  return char_set_node;
}

// list_item ::= character_item
//           |   char_class_item
//           |   char_range_item
//
CharSetItem ParseTree::list_item() {
  if (scanner.is_char_range()) {
    return char_range_item();
  } else if (scanner.get_type() == CHAR_CLASS) {
    return char_class_item();
  } else {
    return character_item();
  }
}

// character_item ::= CHARACTER
// 	          |   '^'
// 	          |   '$'
//	          |   '-'
//
CharSetItem ParseTree::character_item() {
  CharSetItem char_set_item {};
  char_set_item.type = CHARACTER_ITEM;

  if (scanner.get_type() == CHARACTER) {
    char c = scanner.get_character();
    scanner.advance();
    char_set_item.character = c;
  } else if (scanner.get_type() == CARET) {
    scanner.advance();
    char_set_item.character = '^';
  } else if (scanner.get_type() == DOLLAR) {
    scanner.advance();
    char_set_item.character = '$';
  } else if (scanner.get_type() == HYPHEN) {
    scanner.advance();
    char_set_item.character = '-';
  } else {
    std::stringstream s;
    s << "ERROR (parse error): expected character type but received "
      << scanner.get_type_str();
    throw EgretException(s.str());
  }
  char c = char_set_item.character;
  if (ispunct(c)) {
    if (punct_marks.find(c) == punct_marks.end()) {
      punct_marks.insert(c);
    }
  }
  return char_set_item;
}

// char_class_item ::= CHAR_CLASS
//
CharSetItem ParseTree::char_class_item() {
  CharSetItem char_set_item {};
  char_set_item.type = CHAR_CLASS_ITEM;
  char_set_item.character = scanner.get_character();
  scanner.advance();
  return char_set_item;
}

// char_range_item ::= CHARACTER '-' CHARACTER
//
CharSetItem ParseTree::char_range_item() {
  CharSetItem char_set_item {};
  char_set_item.type = CHAR_RANGE_ITEM;

  // TODO:  These seem like sanity checks - maybe assertions instead?
  if (scanner.get_type() != CHARACTER) {
    std::stringstream s;
    s << "ERROR (parse error): expected character type but received "
      << scanner.get_type_str();
    throw EgretException(s.str());
  }
  char start = scanner.get_character();
  scanner.advance();

  if (scanner.get_type() != HYPHEN) {
    std::stringstream s;
    s << "ERROR (parse error): expected hyphen but received "
      << scanner.get_type_str();
    throw EgretException(s.str());
  }
  scanner.advance();

  if (scanner.get_type() != CHARACTER) {
    std::stringstream s;
    s << "ERROR (parse error): expected character type but received "
      << scanner.get_type_str();
    throw EgretException(s.str());
  }
  char end = scanner.get_character();
  scanner.advance();

  char_set_item.range_start = start;
  char_set_item.range_end = end;
  return char_set_item;
}

void ParseTree::print() {
  std::cout << "Tree:" << std::endl;
  print_tree(root, 0);
  std::cout << std::endl;
}

void ParseTree::print_tree(const std::unique_ptr<ParseNode> &node, unsigned offset) {
  if (!node)
    return;

  for (unsigned int i = 0; i < offset; i++)
    std::cout << " ";

  std::cout << "<";
  switch (node->type) {
  case ALTERNATION_NODE:
    std::cout << "alternation |";
    break;
  case CONCAT_NODE:
    std::cout << "concat";
    break;
  case REPEAT_NODE:
    if (node->repeat_upper == -1)
      std::cout << "repeat {" << node->repeat_lower << ",}";
    else
      std::cout << "repeat {" << node->repeat_lower << "," << node->repeat_upper
                << "}";
    break;
  case GROUP_NODE:
    std::cout << "group";
    break;
  case BACKREFERENCE_NODE:
    std::cout << "backreference ";
    node->backref->print();
    break;
  case IGNORED_NODE:
    std::cout << "ignored";
    break;
  case CHARACTER_NODE:
    std::cout << "character: " << node->character;
    break;
  case CARET_NODE:
    std::cout << "caret ^";
    break;
  case DOLLAR_NODE:
    std::cout << "dollar $";
    break;
  case CHAR_SET_NODE:
    std::cout << "charset [";
    node->char_set->print();
    std::cout << "]";
    break;
  default:
    assert(false);
  }

  std::cout << " @ (" << node->loc.first << "," << node->loc.second << ")>"
            << std::endl;

  print_tree(node->left, offset + 2);
  print_tree(node->right, offset + 2);
}

void ParseTree::add_stats(Stats &stats) {
  ParseTreeStats tree_stats = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  gather_stats(root, tree_stats);
  stats.add("PARSE_TREE", "Alternation nodes", tree_stats.alternation_nodes);
  stats.add("PARSE_TREE", "Concat nodes", tree_stats.concat_nodes);
  stats.add("PARSE_TREE", "Repeat nodes", tree_stats.repeat_nodes);
  stats.add("PARSE_TREE", "Group nodes (unnamed)",
            tree_stats.unnamed_group_nodes);
  stats.add("PARSE_TREE", "Group nodes (named)", tree_stats.named_group_nodes);
  stats.add("PARSE_TREE", "Backreference nodes",
            tree_stats.backreference_nodes);
  stats.add("PARSE_TREE", "Caret nodes", tree_stats.caret_nodes);
  stats.add("PARSE_TREE", "Dollar nodes", tree_stats.dollar_nodes);
  stats.add("PARSE_TREE", "Character nodes", tree_stats.character_nodes);
  stats.add("PARSE_TREE", "Character set nodes (not ^)",
            tree_stats.normal_char_set_nodes);
  stats.add("PARSE_TREE", "Character set nodes (^)",
            tree_stats.complement_char_set_nodes);
  stats.add("PARSE_TREE", "Ignored nodes", tree_stats.ignored_nodes);
}

void ParseTree::gather_stats(const std::unique_ptr<ParseNode> &node, ParseTreeStats &tree_stats) {
  if (!node)
    return;

  switch (node->type) {
  case ALTERNATION_NODE:
    tree_stats.alternation_nodes++;
    break;
  case CONCAT_NODE:
    tree_stats.concat_nodes++;
    break;
  case REPEAT_NODE:
    tree_stats.repeat_nodes++;
    break;
  case GROUP_NODE:
    if (node->group_name == "")
      tree_stats.unnamed_group_nodes++;
    else
      tree_stats.named_group_nodes++;
    break;
  case BACKREFERENCE_NODE:
    tree_stats.backreference_nodes++;
    break;
  case CHARACTER_NODE:
    tree_stats.character_nodes++;
    break;
  case CARET_NODE:
    tree_stats.caret_nodes++;
    break;
  case DOLLAR_NODE:
    tree_stats.dollar_nodes++;
    break;
  case CHAR_SET_NODE:
    if (node->char_set->is_complement())
      tree_stats.complement_char_set_nodes++;
    else
      tree_stats.normal_char_set_nodes++;
    break;
  case IGNORED_NODE:
    tree_stats.ignored_nodes++;
    break;
  default:
    assert(false);
  }

  gather_stats(node->left, tree_stats);
  gather_stats(node->right, tree_stats);
}
