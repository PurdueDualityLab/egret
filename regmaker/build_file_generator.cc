//
// Created by charlie on 12/2/22.
//

#include "build_file_generator.h"

#include <stdexcept>
#include <fstream>

#include "fmt/core.h"

BuildFileGenerator::BuildFileGenerator(const std::string &test_template_path) {
  std::ifstream t(test_template_path);
  if (!t.is_open()) {
    throw std::runtime_error("Could not open template file");
  }

  this->test_template = std::string((std::istreambuf_iterator<char>(t)),
                                        std::istreambuf_iterator<char>());
}

void BuildFileGenerator::generate(const RegressionSpec &spec,
                                  std::ostream &output) {
  output << fmt::format(this->test_template, spec.name, spec.name);
}
