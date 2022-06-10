#ifndef CINE2_PARAMETER_H_INCLUDED
#define CINE2_PARAMETER_H_INCLUDED

#include <string>
#include "cmd_line.h"
namespace cine2 {




  struct Param
  {
    int G;                // generations
    int t_scenes;                // time ticks per generation

    int pop_size;
    double initial_comp;
    double act;
    double mutation_rate;
    double mutation_shape;
    double cost_comp;
    double base_intake;

    int dims;
    double changerate;
    int alpha;
    double resource_min;
    double resource_max;

    std::string outdir;   // output folder

  };


  Param parse_parameter(cmd::cmd_line_parser& clp);
  cmd::cmd_line_parser config_file_parser(const std::string& config);

  // write as textfile
  std::ostream& stream_parameter(std::ostream& os,
    const Param& param,
    const char* prefix,
    const char* postfix,
    const char* lb,
    const char* rb);


}
#endif
