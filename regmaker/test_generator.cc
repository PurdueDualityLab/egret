//
// Created by charlie on 12/2/22.
//

#include "test_generator.h"

#include "fmt/core.h"
#include <fstream>
#include <nlohmann/json.hpp>

TestGenerator::TestGenerator(const std::string &template_path) {
  std::ifstream t(template_path);
  if (!t.is_open()) {
    throw std::runtime_error("Could not open template file");
  }

  this->template_contents = std::string((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
}


void TestGenerator::generate(const RegressionSpec &spec, std::ostream &output) {
  // Turn the regex into an escaped string
  nlohmann::json output_escaped(spec.regex);
  output << fmt::format(this->template_contents, spec.name, output_escaped.dump());
}
