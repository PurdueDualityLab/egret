//
// Created by charlie on 12/2/22.
//

#ifndef EGRET_REGRESSION_SPEC_H
#define EGRET_REGRESSION_SPEC_H

#include <string>

struct RegressionSpec {
  RegressionSpec(std::string name, std::string regex)
      : name(std::move(name))
      , regex(std::move(regex))
  {}

  std::string filename() const {
    return this->name + ".cc";
  }

  std::string name;
  std::string regex;
};

#endif // EGRET_REGRESSION_SPEC_H
