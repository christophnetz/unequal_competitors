#pragma once
#define NOMINMAX
#include "parameter.h"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <numeric>
#include <fstream>
#include <algorithm>
#include <string>
#include "rnd.hpp"


using namespace cine2;
using namespace std;


template <typename T>
class Grid {
public:
  explicit Grid(int dim) : dim_(dim), buf_(size_t(dim)* dim) {}
  Grid(int dim, const T& val) : dim_(dim), buf_(size_t(dim)* dim, val) {}

  // access to underlying linear buffer
  const auto& buf() const noexcept { return buf_; }
  auto& buf() noexcept { return buf_; }

  // 2D stuff
  int dim() const noexcept { return dim_; }
  auto& operator()(int x, int y) { return buf_[size_t(y) * dim_ + x]; }
  const auto& operator()(int x, int y) const { return buf_[size_t(y) * dim_ + x]; }

  // conversion linear <-> 2D
  size_t linear_idx(int x, int y) const noexcept { return size_t(y) * dim_ + x; }
  std::pair<int, int> coor(size_t idx) const noexcept {
    return { static_cast<int>(idx) % dim_, static_cast<int>(idx) / dim_ };
  }

private:
  int dim_;
  std::vector<T> buf_;   // linear buffer
};



using landscape_t = Grid<double>;
using presence_t = Grid<double>;



struct ind {

  ind() {}
  ind(int x, int y, double c) : xpos(x), ypos(y), comp(c) {}

  void move(const landscape_t& landscape, presence_t& presence, Param param_);
  double updateintake(const landscape_t& landscape, presence_t& presence);

  double comp;
  int xpos;
  int ypos;
};


double ind::updateintake(const landscape_t& landscape, presence_t& presence) {
  return landscape(xpos, ypos) * comp / presence(xpos, ypos);
}


void ind::move(const landscape_t& landscape, presence_t& presence, Param param_) {

  double present_intake = landscape(xpos, ypos) * comp / (presence(xpos, ypos));
  double potential_intake;
  int former_xpos = xpos;
  int former_ypos = ypos;

  auto bestidx = landscape.linear_idx(xpos, ypos);
  for (int i = 0; i < landscape.buf().size(); ++i) {
    const auto idx = i;
    potential_intake = landscape.buf()[idx] * comp / (presence.buf()[idx] + comp);
    if (present_intake < potential_intake) {
      present_intake = potential_intake;
      bestidx = idx;
    }
  }

  const auto newpos = landscape.coor(bestidx);
  xpos = newpos.first;
  ypos = newpos.second;
  presence.buf()[bestidx] += comp;
  presence(former_xpos, former_ypos) -= comp;
}



bool check_IFD(const vector<ind>& pop, const landscape_t& landscape, const presence_t& presence) {

  for (const auto& ind : pop) {
    const auto present_intake = landscape(ind.xpos, ind.ypos) * ind.comp / presence(ind.xpos, ind.ypos);
    for (int i = 0; i < landscape.buf().size(); ++i) {
      if (present_intake < landscape.buf()[i] * ind.comp / (presence.buf()[i] + ind.comp)) {
        return false;
      }
    }
  }
  return true;
}



void rand_to_IFD_opt(const Param& param_) {
  std::ofstream ofs1(param_.outdir + "_ifdopt.txt", std::ofstream::out);
  ofs1 << "itt\tcomp\txpos\typos\tresource\tintake\n";

  uniform_int_distribution<int> pdist(0, param_.dims - 1);

  //Initialize population
  vector<ind> pop;

  for (int i = 0; i < param_.pop_size; ++i) {
    pop.emplace_back(pdist(rnd::reng), pdist(rnd::reng), param_.types[ i / (param_.pop_size/param_.types.size())]);
  }

  //Initialize patches  
  landscape_t landscape(param_.dims);

  for (int i = 0; i < landscape.buf().size(); ++i) {
    landscape.buf()[i] = (i + 1) * 0.01;
  }

  
  for (int j = 0; j < param_.repeats; ++j) {

    // redistribute individuals and update presence record
    presence_t presence(param_.dims, 0.0);
    for (int i = 0; i < pop.size(); ++i) {
      pop[i].xpos = pdist(rnd::reng);
      pop[i].ypos = pdist(rnd::reng);
      presence(pop[i].xpos, pop[i].ypos) += pop[i].comp;
    }

    bool IFD_reached = false;
    int it_t = 0;


    while (!IFD_reached) { //otherwise, and if IFD is not reached yet, let individual move

      int id = std::uniform_int_distribution<int>(0, pop.size() - 1)(rnd::reng);
      pop[id].move(landscape, presence, param_);

      // IFD checking in certain intervalls
      if (it_t % 100 == 0) {
        IFD_reached = check_IFD(pop, landscape, presence);
      }
      it_t++;
    }

    // Output
    for (int i = 0; i < pop.size(); ++i) {
      ofs1 << j << "\t" << pop[i].comp << "\t" << pop[i].xpos << "\t" << pop[i].ypos << "\t"
        << landscape(pop[i].xpos, pop[i].ypos) << "\t" <<pop[i].updateintake(landscape, presence) << "\n" ;
    }

    cout << j << "\n";
  }
}


void simulation(const Param& param_) {

  if (param_.seed != 0)
    rnd::reng.seed(param_.seed);

  //Write out parameters
  std::ofstream os(param_.outdir + "params.txt", std::ofstream::out);
  stream_parameter(os, param_, "  ", ",\n", "c(", ")");
  os.close();


  rand_to_IFD_opt(param_);

  cout << "End";

}
