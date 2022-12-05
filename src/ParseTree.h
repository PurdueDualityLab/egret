/*  ParseTree.h: recursive descent parser

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

// The BNF for our simple regexes is:
//
// expr		::= concat '|' expr		(expression)
// 		|   concat '|'
//		|   '|' expr
//		|   '|'
//		|   concat
//
// concat	::= rep concat			(concatenation)
// 		|   rep
//
// rep		::= atom '*'			(repetition)
// 		|   atom '+'
// 		|   atom '?'
// 		|   atom '{n,m}'
// 		|   atom '{n,}'
// 		|   atom
//
// atom		::= group			(atomic)
// 		|   character
// 		|   char_class
// 		|   char_set
//
// group	::= '(' expr ')'		(group)
// 		| '(' NO_GROUP_EXT expr ')'
// 		| '(' NAMED_GROUP_EXT expr ')'
// 		| '(' IGNORED_EXT expr ')'
// 		| '(' IGNORED_EXT ')'
//
// character	::= CHARACTER 			(character)
//		|   '^'
//		|   '$'
//		|   '-'
//		|   WORD_BOUNDARY
//
// char_class	::= CHAR_CLASS			(character class)
//
// char_set	::= '[' char_list ']'		(character set)
// 		|   '[' '^' char_list ']'
//
// char_list	::= list_item char_list		(character list)
// 		|   list_item
//
// list_item	::= character_item		(list item)
//		|   char_class_item
// 		|   char_range_item
//
// character_item ::= CHARACTER			(character item)
//		|   '^'
//		|   '$'
//		|   '-'
//
// char_class_item ::= CHAR_CLASS		(character class item)
//
// char_range_item ::= CHARACTER '-' CHARACTER	(character range item)
//

#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include "Backref.h"
#include "CharSet.h"
#include "Scanner.h"
#include "Stats.h"
#include <cassert>
#include <set>
#include <unordered_map>
#include <memory>
#include <utility>

typedef enum {
  ALTERNATION_NODE,
  CONCAT_NODE,
  REPEAT_NODE,
  GROUP_NODE,
  BACKREFERENCE_NODE,
  CHARACTER_NODE,
  CHAR_SET_NODE,
  CARET_NODE,
  DOLLAR_NODE,
  IGNORED_NODE
} NodeType;

struct ParseNode {
  ParseNode(NodeType t, Location _loc, std::shared_ptr<ParseNode> l, std::shared_ptr<ParseNode> r)
  : type(t)
  , loc(std::move(_loc))
  , left(std::move(l))
  , right(std::move(r))
  , character(0)
  , repeat_lower(-1)
  , repeat_upper(-1) {
  }

  ParseNode(NodeType t, Location _loc, std::string _name, std::shared_ptr<ParseNode> l,
            std::shared_ptr<ParseNode> r)
  : type(t)
  , loc(std::move(_loc))
  , left(std::move(l))
  , right(std::move(r))
  , character(0)
  , repeat_lower(-1)
  , repeat_upper(-1)
  , group_name(std::move(_name)) {
    assert(t == GROUP_NODE);
  }

  ParseNode(NodeType t, Location _loc, std::shared_ptr<CharSet> c)
  : type(t)
  , loc(std::move(_loc))
  , left()
  , right()
  , character(0)
  , char_set(std::move(c))
  , repeat_lower(-1)
  , repeat_upper(-1) {
    assert(t == CHAR_SET_NODE);
  }

  ParseNode(NodeType t, Location _loc, char c)
  : type(t)
  , loc(std::move(_loc))
  , character(c)
  , repeat_lower(-1)
  , repeat_upper(-1) {
    assert(t == CHARACTER_NODE);
  }

  ParseNode(NodeType t, Location _loc, std::shared_ptr<Backref> b)
  : type(t)
  , loc(std::move(_loc))
  , character(0)
  , repeat_lower(-1)
  , repeat_upper(-1)
  , backref(std::move(b)) {
    assert(t == BACKREFERENCE_NODE);
  }

  ParseNode(NodeType t, Location _loc, std::shared_ptr<ParseNode> l, int lower, int upper)
  : type(t)
  , loc(std::move(_loc))
  , left(std::move(l))
  , character(0)
  , repeat_lower(lower)
  , repeat_upper(upper) {
    assert(t == REPEAT_NODE);
  }

  NodeType type;
  Location loc;
  std::shared_ptr<ParseNode> left;
  std::shared_ptr<ParseNode> right;
  char character;         // For CHARACTER_NODE
  std::shared_ptr<CharSet> char_set;      // For CHAR_SET_NODE
  int repeat_lower;       // For REPEAT_NODE
  int repeat_upper;       // For REPEAT_NODE (-1 for no limit)
  std::shared_ptr<Backref> backref;       // For BACKREFERENCE_NODE
  std::string group_name; // For GROUP_NODE
};

class ParseTree {

public:
  // build parse tree using regex stored in scanner
  void build(Scanner &_scanner);

  // get root of the tree
  std::shared_ptr<ParseNode> get_root() { return root; }

  // get set of punctuation marks
  std::set<char> get_punct_marks() { return punct_marks; }

  // prints the tree
  void print();

  // get tree stats
  void add_stats(Stats &stats);

private:
  std::shared_ptr<ParseNode> root;            // root of parse tree
  Scanner scanner;            // scanner
  std::set<char> punct_marks; // set of punctuation marks
  std::unordered_map<int, Location> group_locs;
  std::unordered_map<std::string, Location> named_group_locs;
  int group_count;

  // creation functions
  std::shared_ptr<ParseNode> expr();
  std::shared_ptr<ParseNode> concat();
  std::shared_ptr<ParseNode> rep();
  std::shared_ptr<ParseNode> atom();
  std::shared_ptr<ParseNode> group();
  std::shared_ptr<ParseNode> character();
  std::shared_ptr<ParseNode> char_class();
  std::shared_ptr<ParseNode> char_set();
  std::shared_ptr<ParseNode> char_list(int start_loc);
  CharSetItem list_item();
  CharSetItem character_item();
  CharSetItem char_class_item();
  CharSetItem char_range_item();

  // print the tree
  void print_tree(const std::shared_ptr<ParseNode> &node, unsigned offset);

  // gather stats
  struct ParseTreeStats {
    int alternation_nodes;
    int concat_nodes;
    int repeat_nodes;
    int unnamed_group_nodes;
    int named_group_nodes;
    int backreference_nodes;
    int character_nodes;
    int caret_nodes;
    int dollar_nodes;
    int normal_char_set_nodes;
    int complement_char_set_nodes;
    int ignored_nodes;
  };
  void gather_stats(const std::shared_ptr<ParseNode> &node, ParseTreeStats &tree_stats);
};

#endif // PARSE_TREE_H
