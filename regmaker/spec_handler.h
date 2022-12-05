//
// Created by charlie on 12/2/22.
//

#ifndef EGRET_SPEC_HANDLER_H
#define EGRET_SPEC_HANDLER_H

#include "regmaker/regression_spec.h"
#include <string>
#include <vector>

class SpecHandler {
public:
  explicit SpecHandler(const std::string &input_file_path);

  const std::vector<RegressionSpec> &get_spec() const { return specs; }

private:
  std::vector<RegressionSpec> specs;
};

#endif // EGRET_SPEC_HANDLER_H
