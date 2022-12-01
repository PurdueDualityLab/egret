/*  Util.h: Singleton utility class

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

#ifndef ERROR_H
#define ERROR_H

#include <set>
#include <string>
#include <utility>
#include <vector>
#include <memory>

// Location
typedef std::pair<int, int> Location;

struct Alert {

  bool warning;
  std::string type;
  std::string message;
  bool has_suggest;
  std::string suggest;
  bool has_example;
  std::string example;
  Location loc1;
  Location loc2;

  Alert(std::string t, std::string m) {
    warning = false;
    type = std::move(t);
    message = std::move(m);
    has_suggest = false;
    has_example = false;
    loc1 = std::make_pair(-1, -1);
    loc2 = std::make_pair(-1, -1);
  }
  Alert(std::string t, std::string m, Location l1) {
    warning = false;
    type = std::move(t);
    message = std::move(m);
    has_suggest = false;
    has_example = false;
    loc1 = l1;
    loc2 = std::make_pair(-1, -1);
  }
  Alert(std::string t, std::string m, Location l1, Location l2) {
    warning = false;
    type = std::move(t);
    message = std::move(m);
    has_suggest = false;
    has_example = false;
    loc1 = l1;
    loc2 = l2;
  }
  Alert(std::string t, std::string m, std::string s) {
    warning = false;
    type = std::move(t);
    message = std::move(m);
    has_suggest = true;
    has_example = false;
    suggest = std::move(s);
    loc1 = std::make_pair(-1, -1);
    loc2 = std::make_pair(-1, -1);
  }
  Alert(std::string t, std::string m, std::string s, Location l1) {
    warning = false;
    type = std::move(t);
    message = std::move(m);
    has_suggest = true;
    has_example = false;
    suggest = std::move(s);
    loc1 = l1;
    loc2 = std::make_pair(-1, -1);
  }
  Alert(std::string t, std::string m, std::string s, Location l1, Location l2) {
    warning = false;
    type = std::move(t);
    message = std::move(m);
    has_suggest = true;
    has_example = false;
    suggest = std::move(s);
    loc1 = l1;
    loc2 = l2;
  }
};

class Util {

public:
  static std::shared_ptr<Util> get();

  void init(std::string r, bool c, bool w, std::string s);

  bool is_check_mode() const { return check_mode; }
  bool is_web_mode() const { return web_mode; }
  std::string get_base_substring() { return base_substring; }
  std::string get_regex() { return regex; }
  std::vector<std::string> get_alerts() { return alerts; }

  // Alerts
  void add_alert(const Alert& alert);

  // TODO: Possibly create a new regex class where the "fixing" functions
  // reside?
private:
  Util() = default; // singleton class, private constructor
  static std::shared_ptr<Util> inst;

  // Global options
  bool check_mode{};
  bool web_mode{};
  std::string base_substring;

  std::string regex; // original regular expression

  // Alerts
  std::vector<std::string> alerts;                   // vector of alert strings
  std::set<std::pair<std::string, int>> prev_alerts; // all previous alerts
};

// TODO: Can this exception be folded into util class above?
// TODO: One idea is to add an add_error function that throws an exception that
// is caught at the top level. Egret Exception
class EgretException : public std::exception {

public:
  explicit EgretException(std::string msg) noexcept {
    error_msg = std::move(msg);
  }
  EgretException(const EgretException &other) noexcept {
    error_msg = other.error_msg;
  }

  const std::string &get_error() const { return error_msg; }

  ~EgretException() _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override = default;

  const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
    return this->error_msg.c_str();
  }

private:
  std::string error_msg;
};

#endif // ERROR_H
