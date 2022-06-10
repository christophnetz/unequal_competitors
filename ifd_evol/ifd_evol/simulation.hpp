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
#include "cached.hpp"
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


class iota_sampler_t {
public:
  explicit iota_sampler_t(size_t i) : iotas_(i) {
    std::iota(iotas_.begin(), iotas_.end(), 0);
  }

  template <typename Reng>
  const std::vector<int>& operator()(Reng& reng) {
    std::shuffle(iotas_.begin(), iotas_.end(), reng);
    return iotas_;
  }

private:
  std::vector<int> iotas_;
};


struct ind {

  ind() {}
  ind(int x, int y, double c) : xpos(x), ypos(y), comp(c) {}

  void mutate(bernoulli_distribution& mutate, normal_distribution<double>& mshape);

  void move(const landscape_t& landscape, presence_t& presence, Param param_);

  void springoff(const ind& parent);
  double updateintake(const landscape_t& landscape, presence_t& presence);


  double food = 0.0;
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
  for (int i = 0; i < param_.dims * param_.dims; ++i) {
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


void ind::mutate(bernoulli_distribution& mrate, normal_distribution<double>& mshape) {

  if (mrate(rnd::reng)) {
    comp += mshape(rnd::reng);
    comp = max(comp, 0.1);  // Can't be 0, better way to implement?
  }
}

void ind::springoff(const ind& parent) {
  food = 0.0;
  comp = parent.comp;
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


void landscape_setup(landscape_t& landscape, Param param_) {
  for (double& c : landscape.buf()) {
    c = uniform_real_distribution<double>(param_.resource_min, param_.resource_max)(rnd::reng);
  }
}

void reproduction(vector<ind>& pop, vector<ind>& tmp_pop, const Param& param_) {

  rndutils::mutable_discrete_distribution<int, rndutils::all_zero_policy_uni> rdist;
  rdist.mutate_transform(pop.cbegin(), pop.cend(), [&](const ind& i) {
    return max(0.0, param_.base_intake + i.food - param_.cost_comp * i.comp * param_.t_scenes);});

  uniform_int_distribution<int> pdist(0, param_.dims - 1);
  bernoulli_distribution mrate(param_.mutation_rate);
  normal_distribution<double> mshape(0.0, param_.mutation_shape);

  for (int i = 0; i < pop.size(); ++i) {
    const int ancestor = rdist(rnd::reng);
    tmp_pop[i].springoff(pop[ancestor]);
    tmp_pop[i].xpos = pdist(rnd::reng);
    tmp_pop[i].ypos = pdist(rnd::reng);
    tmp_pop[i].mutate(mrate, mshape);
  }
  pop.swap(tmp_pop);
}

void ifdnoifd(const Param& param_, landscape_t landscape, vector<ind> pop) {

  std::ofstream ofs_ifd(param_.outdir + "_ifd.txt", std::ofstream::out);
  ofs_ifd << "i\tIFD\tcomp\tintake\n";

  uniform_int_distribution<int> pdist(0, param_.dims - 1);

  bool IFD_reached = false;

  for (int j = 0; j < 1000; ++j) {
    presence_t presence(param_.dims, 0.0);
    landscape_setup(landscape, param_);

    for (int i = 0; i < pop.size(); ++i) {
      pop[i].xpos = pdist(rnd::reng);
      pop[i].ypos = pdist(rnd::reng);
      presence(pop[i].xpos, pop[i].ypos) += pop[i].comp;
    }

    int it_t = 0;

    while (!IFD_reached) { //otherwise, and if IFD is not reached yet, let individual move
      int id = std::uniform_int_distribution<int>(0, pop.size() - 1)(rnd::reng);

      pop[id].move(landscape, presence, param_);

      // IFD checking in certain intervalls, costly and to be omitted?
      if (it_t % 100 == 0) {
        IFD_reached = check_IFD(pop, landscape, presence);
      }
      it_t++;
    }

    for (int i = 0; i < pop.size(); ++i) {
      ofs_ifd << j << "\t" << IFD_reached << "\t" << pop[i].comp <<
        "\t" << pop[i].updateintake(landscape, presence) << "\n";
    }

    landscape_setup(landscape, param_);
    IFD_reached = false;

    for (int i = 0; i < pop.size(); ++i) {
      ofs_ifd << j << "\t" << IFD_reached << "\t" << pop[i].comp <<
        "\t" << pop[i].updateintake(landscape, presence) << "\n";
    }
  }

  ofs_ifd.close();
}



void simulation(const Param& param_) {



  //Output filestreams
  std::ofstream ofs1(param_.outdir + "comp.txt", std::ofstream::out);
  std::ofstream ofs2(param_.outdir + "_landscape.txt", std::ofstream::out);
  ofs2 << "gen\ttime\tlastchange\tcomp\txpos\typos\tfood\tintake\n";


  std::ofstream os(param_.outdir + "params.txt", std::ofstream::out);
  stream_parameter(os, param_, "  ", ",\n", "c(", ")");
  os.close();

  //landscape initialization
  landscape_t landscape(param_.dims);


  //Population initialization
  vector<ind> pop;
  vector<ind> tmp_pop(param_.pop_size);
  auto pdist = std::uniform_int_distribution<int>(0, param_.dims - 1);
  for (int i = 0; i < param_.pop_size; ++i) {
    pop.emplace_back(pdist(rnd::reng), pdist(rnd::reng), param_.initial_comp);
  }
  auto iota_sampler = cached_iota(landscape.buf().size(), 100'000);

  // Set up gillespie algorithm
  vector<double> activities(param_.pop_size, param_.act);
  activities.push_back(param_.changerate); // Add rate of environmental change

  auto rdist = cached_rdist(100'000);
  rdist.mutate(activities.cbegin(), activities.cend());
  double total_act = std::accumulate(activities.begin(), activities.end(), 0.0);
  auto event_dist = cached_exponential(total_act, 100'000);

  presence_t presenceNull(param_.dims, 0.0);
  presence_t presence(param_.dims, 0.0);

  // Generation loop
  for (int g = 0; g < param_.G; ++g) {

    // Update presences
    presence = presenceNull;
    for (int i = 0; i < pop.size(); ++i) {
      presence(pop[i].xpos, pop[i].ypos) += pop[i].comp;
    }

    // Set up new landscape
    landscape_setup(landscape, param_);

    //Some tracking variables
    double time = 0.0;
    int id;
    double eat_t = 0.0;
    double it_t = 0.0;
    double increment = 0.1;
    bool IFD_reached = false;
    int count = 0;
    double last_change = 0.0;

    for (; time < param_.t_scenes; ) {

      time += event_dist(); // time to next event

      while (time > eat_t) { // alternative: individuals eat continuously. Maybe let's not

        for (int p = 0; p < pop.size(); ++p) {

          pop[p].food += landscape(pop[p].xpos, pop[p].ypos) * pop[p].comp / presence(pop[p].xpos, pop[p].ypos);
        }

        // landscape output in last generation
        if (g == param_.G - 1) {

          for (int i = 0; i < pop.size(); ++i) {

            ofs2 << g << "\t" << eat_t << "\t" << last_change << "\t" << pop[i].comp << "\t" << pop[i].xpos << "\t"
              << pop[i].ypos << "\t" << pop[i].food << "\t" << pop[i].updateintake(landscape, presence) << "\n";

          }
        }

        eat_t += 1.0;

      }

      // Choose which ind moves
      id = rdist();
      if (id == pop.size()) //if last one, environmental change counter is incremented
        ++count;

      else if (!IFD_reached) { //otherwise, and if IFD is not reached yet, let individual move

        pop[id].move(landscape, presence, param_);

        // IFD checking in certain intervalls, costly 
        if (time > it_t) {
          IFD_reached = check_IFD(pop, landscape, presence);
          it_t = floor(time / increment) * increment + increment;
        }
      }

      //Change landscape
      if (count == param_.alpha)
      {
        //resetting
        IFD_reached = false;
        count = 0;
        last_change = time;


        const auto& iota = iota_sampler();
        for (int i = 0; i < param_.dims * param_.dims; ++i) {
          landscape.buf()[iota[i]] = uniform_real_distribution<double>(param_.resource_min, param_.resource_max)(rnd::reng);
        }
      }
    }


    ofs1 << g << "\t";

    for (int q = 0; q < pop.size(); q += max(1, static_cast<int>(pop.size() / 1000))) {
      ofs1 << pop[q].comp << "\t";
    }
    ofs1 << "\n";

    reproduction(pop, tmp_pop, param_);
    cout << g << endl;
  }


  ifdnoifd(param_, landscape, pop);


  ofs1.close();
  ofs2.close();

  cout << rnd::reng <<"End";

}
