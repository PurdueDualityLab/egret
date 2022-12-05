//
// Created by charlie on 12/2/22.
//

#include "spec_handler.h"

#include <fstream>
#include "nlohmann/json.hpp"

SpecHandler::SpecHandler(const std::string &input_file_path) {

  std::ifstream input(input_file_path);
  if (!input.is_open()) {
    throw std::runtime_error("SpecHandler: Could not open input file");
  }

#if 0
  // Parse the json
  nlohmann::json file_obj;
  input >> file_obj;

  for (const auto &spec_obj : file_obj) {
    auto name = spec_obj["name"].get<std::string>();
    auto pattern = spec_obj["pattern"].get<std::string>();

    // Add a new spec
    this->specs.emplace_back(name, pattern);
  }
#else
  std::string line;
  unsigned long encountered = 1;
  while (std::getline(input, line)) {
    nlohmann::json obj = nlohmann::json::parse(line);
    auto pattern = obj["pattern"].get<std::string>();
    auto name = std::string("regtest") + std::to_string(encountered++);

    this->specs.emplace_back(name, pattern);
  }
#endif
  // DONE
}
