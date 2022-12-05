//
// Created by charlie on 12/2/22.
//

#include "regmaker/build_file_generator.h"
#include "regmaker/spec_handler.h"
#include "regression_spec.h"
#include "test_generator.h"
#include <fstream>
#include <iostream>
#include <fmt/core.h>

int main(int argc, char **argv) {

  // Test it out
  /*
  RegressionSpec spec("test1", "[a-z]+");
  std::ofstream test_output("/home/charlie/Programming/egret/src/test/regression/test1.cc");
  std::ofstream build_output("/home/charlie/Programming/egret/src/test/regression/BUILD.bazel");
  TestGenerator generator("/home/charlie/Programming/egret/regmaker/regression-test.template");
  BuildFileGenerator build_generator("/home/charlie/Programming/egret/regmaker/test_spec.template");


  generator.generate(spec, test_output);
  build_generator.generate(spec, build_output);
   */

  if (argc < 2) {
    std::cerr << "usage: regmaker <patterns file>" << std::endl;
    return 1;
  }

  // Load regression specs
  SpecHandler spec_handler(argv[1]);
  // Set up generators
  TestGenerator test_generator("/home/charlie/Programming/egret/regmaker/regression-test.template");
  BuildFileGenerator build_generator("/home/charlie/Programming/egret/regmaker/test_spec.template");
  // Create BUILD file
  std::ofstream build_output("/home/charlie/Programming/egret/src/test/regression/BUILD.bazel");

  for (const auto &spec : spec_handler.get_spec()) {
    // Generate the test file
    std::ofstream test_output("/home/charlie/Programming/egret/src/test/regression/" + spec.filename());
    test_generator.generate(spec, test_output);
    // Add the entry to the build file
    build_generator.generate(spec, build_output);
    build_output << std::endl;
  }

  // DONE

  return 0;
}
