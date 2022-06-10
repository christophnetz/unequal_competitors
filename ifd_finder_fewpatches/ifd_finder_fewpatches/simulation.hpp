#pragma once
#define NOMINMAX
#include "parameter.h"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <numeric>
#include <fstream>
#include <algorithm>
#include <string>
#include "rnd.hpp"


using namespace cine2;
using namespace std;



struct ind {

  ind() {}
  ind(int p, double c) : pos(p), comp(c) {}

  void move(const vector<double>& landscape, vector<double>& presence, vector<int>& numpresence, Param param_);

  double comp;
  int pos;
};


void ind::move(const vector<double>& landscape, vector<double>& presence, vector<int>& numpresence, Param param_) {

  double present_intake;
  double potential_intake;
  int former_pos = pos;
  int bestidx;
  if (pos == -1) {
    present_intake = 0;
    bestidx = 0;
  }
  else {
    present_intake = landscape[pos] * comp / (presence[pos]);
    bestidx = pos;
  }


  for (int i = 0; i < landscape.size(); ++i) {
    const auto idx = i;
    potential_intake = landscape[idx] * comp / (presence[idx] + comp);
    if (present_intake < potential_intake) {
      present_intake = potential_intake;
      bestidx = idx;
    }
  }


  const auto newpos = bestidx;
  pos = newpos;
  presence[bestidx] += comp;
  numpresence[bestidx]++;
  if (former_pos >= 0) {
    presence[former_pos] -= comp;
    numpresence[former_pos]--;
  }
}



bool check_IFD(const vector<ind>& pop, const vector<double>& landscape, const vector<double>& presence) {

  for (const auto& ind : pop) {
    if (ind.pos < 0)
      return false;
    else {
      const auto present_intake = landscape[ind.pos] * ind.comp / presence[ind.pos];
      for (int i = 0; i < landscape.size(); ++i) {
        if (present_intake < landscape[i] * ind.comp / (presence[i] + ind.comp) || ind.pos < 0) {
          return false;
        }
      }
    }
  }
  return true;
}



void rand_to_IFD_opt2patchoutside(const Param& param_) {

  std::ofstream ofs1(param_.outdir + "_ifdopt2patchoutside.txt", std::ofstream::out);

  ofs1 << "sim\tcellnr\tnumpresence\tpresence\tresources\n";
  std::vector<double> landscape;
  std::vector<double> presence(param_.patches, 0.0);
  std::vector<int> numpresence(param_.patches, 0);


  for (int i = 0; i < param_.patches; ++i) {
    landscape.emplace_back((1.0 + static_cast<double>(i / (param_.patches / 2))));
  }

  double resourcetotal = std::accumulate(landscape.begin(), landscape.end(), 0.0);


  for (int k = 0; k < param_.repeats; ++k) {

    vector<ind> pop;
    std::fill(presence.begin(), presence.end(), 0.0);
    std::fill(numpresence.begin(), numpresence.end(), 0);

    for (int i = 0; i < param_.pop_size; ++i) {
      pop.emplace_back(-1, 1 + i / (param_.pop_size / param_.types));
    }

    bool IFD_reached = false;
    int it_t = 0;

    while (!IFD_reached) { //otherwise, and if IFD is not reached yet, let individual move
      int id = std::uniform_int_distribution<int>(0, pop.size() - 1)(rnd::reng);

      pop[id].move(landscape, presence, numpresence, param_);


      if (it_t % 100 == 0) {
        IFD_reached = check_IFD(pop, landscape, presence);
      }
      it_t++;
    }

    cout <<"extern\t"<< k << "\n";

    for (int i = 0; i < landscape.size(); ++i) {

      ofs1 << param_.outdir << "\t" << i << "\t" << numpresence[i] << "\t" << presence[i] <<
        "\t" << landscape[i] << "\n";
    }
  }
  ofs1.close();
}


void rand_to_IFD_opt2patch(const Param& param_) {

  std::ofstream ofs1(param_.outdir + "_ifdopt2patch.txt", std::ofstream::out);

  ofs1 << "sim\tcellnr\tnumpresence\tpresence\tresources\n";
  uniform_int_distribution<int> pdist(0, param_.patches - 1);
  std::vector<double> landscape;
  std::vector<double> presence(param_.patches, 0.0);
  std::vector<int> numpresence(param_.patches, 0);


  for (int i = 0; i < param_.patches; ++i) {
    landscape.emplace_back((1.0 + static_cast<double>(i / (param_.patches / 2))));
  }


  double resourcetotal = std::accumulate(landscape.begin(), landscape.end(), 0.0);


  for (int k = 0; k < param_.repeats; ++k) {

    vector<ind> pop;
    std::fill(presence.begin(), presence.end(), 0.0);
    std::fill(numpresence.begin(), numpresence.end(), 0);

    for (int i = 0; i < param_.pop_size; ++i) {
      pop.emplace_back(pdist(rnd::reng), 1 + i / (param_.pop_size / param_.types));
      presence[pop[i].pos] += pop[i].comp;
      numpresence[pop[i].pos]++;
    }


    bool IFD_reached = false;
    int it_t = 0;


    while (!IFD_reached) { //otherwise, and if IFD is not reached yet, let individual move
      int id = std::uniform_int_distribution<int>(0, pop.size() - 1)(rnd::reng);

      pop[id].move(landscape, presence, numpresence, param_);

      if (it_t % 100 == 0) {
        IFD_reached = check_IFD(pop, landscape, presence);
      }
      it_t++;
    }

    cout << "intern\t" << k << "\n";


    for (int i = 0; i < landscape.size(); ++i) {
      ofs1 << param_.outdir << "\t" << i << "\t" << numpresence[i] << "\t" << presence[i] <<
        "\t" << landscape[i] << "\n";
    }
  }
  ofs1.close();
}


void simulation(const Param& param_) {

  if (param_.seed != 0)
    rnd::reng.seed(param_.seed);

  std::ofstream os(param_.outdir + "params.txt", std::ofstream::out);
  stream_parameter(os, param_, "  ", ",\n", "c(", ")");
  os.close();

  rand_to_IFD_opt2patch(param_);
  rand_to_IFD_opt2patchoutside(param_);

  cout << "End";
}
