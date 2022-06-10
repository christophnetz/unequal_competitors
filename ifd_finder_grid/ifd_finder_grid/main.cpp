#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <numeric>
#include <fstream>
#include <algorithm>
#include <string>

#include "rndutils.hpp"
#include "cmd_line.h"
#include "parameter.h"
#include "simulation.hpp"

using namespace std;
using namespace cine2;



int main(int argc, const char** argv) {

  cmd::cmd_line_parser clp(argc, argv);
  try {

    auto config = clp.optional_val("config", std::string{});
    if (!config.empty()) clp.append(config_file_parser(config));
    Param param = parse_parameter(clp);
    auto unknown = clp.unrecognized();
    if (!unknown.empty()) {
      for (const auto& arg : unknown) {
        std::cerr << "unknown argument '" << arg << "'\n";
      }
      return 1;
    }

    simulation(param);

  }

  catch (cmd::parse_error & err) {
    std::cerr << "\nParameter trouble: " << err.what() << '\n';
  }
  catch (std::exception & err) {
    std::cerr << "\nExeption caught: " << err.what() << '\n';
  }
  catch (...) {
    std::cerr << "\nUnknown exeption caught\n";
  }

  return 0;
}