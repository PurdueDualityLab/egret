//
// Created by charlie on 12/2/22.
//

#ifndef EGRET_TEST_GENERATOR_H
#define EGRET_TEST_GENERATOR_H

#include "regmaker/regression_spec.h"
#include <string>
class TestGenerator {
public:
  explicit TestGenerator(const std::string &template_path);

  void generate(const RegressionSpec &spec, std::ostream &output);
private:
  std::string template_contents;
};

#endif // EGRET_TEST_GENERATOR_H
