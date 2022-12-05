//
// Created by charlie on 12/2/22.
//

#ifndef EGRET_BUILD_FILE_GENERATOR_H
#define EGRET_BUILD_FILE_GENERATOR_H

#include "regmaker/regression_spec.h"
#include <string>

class BuildFileGenerator {
public:
  explicit BuildFileGenerator(const std::string &test_template_path);

  void generate(const RegressionSpec &spec, std::ostream &output);

private:
  std::string test_template;
};

#endif // EGRET_BUILD_FILE_GENERATOR_H
