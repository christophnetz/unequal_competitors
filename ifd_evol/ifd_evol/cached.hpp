#pragma once

// cached stuff pre-computed in parallel
// Hanno 2021


#include <atomic>
#include <vector>
#include <algorithm>
#include <numeric>
#include <tbb/tbb.h>
#include "rndutils.hpp"


class cached_exponential {
public:
  cached_exponential(double arg, size_t cache_size) :
    pivot_(cache_size),
    buf_(cache_size),
    arg_(arg)
  {}

  double operator()() {
    if (pivot_ == buf_.size()) {
      fill();
      pivot_ = 0;
    }
    return buf_[pivot_++];
  }

private:
  void fill() {
    tbb::parallel_for(tbb::blocked_range<size_t>{ 0ull, buf_.size() }, [&](const auto range) {
      auto reng = rndutils::make_random_engine_low_entropy<>();
      auto dist = std::exponential_distribution<>(arg_);
      for (auto i = range.begin(); i < range.end(); ++i) {
        buf_[i] = dist(reng);
      }
    });
  }
  
  size_t pivot_;
  std::vector<double> buf_;
  double arg_;
};



class cached_iota {
public:
  cached_iota(size_t n, size_t cache_size) :
    pivot_(cache_size)
  {
    std::vector<int> tmp(n);
    std::iota(tmp.begin(), tmp.end(), 0);
    buf_.resize(cache_size, tmp);
  }

  const std::vector<int>& operator()() {
    if (pivot_ == buf_.size()) {
      fill();
      pivot_ = 0;
    }
    return buf_[pivot_++];
  }

private:
  void fill() {
    tbb::parallel_for(tbb::blocked_range<size_t>{ 0ull, buf_.size() }, [&](const auto range) {
      auto reng = rndutils::make_random_engine_low_entropy<>();
      for (auto i = range.begin(); i < range.end(); ++i) {
        std::shuffle(buf_[i].begin(), buf_[i].end(), reng);
      }
    });
  }

  size_t pivot_;
  std::vector<std::vector<int>> buf_;
};


class cached_rdist {
public:
  using rdist_t = rndutils::mutable_discrete_distribution<int, rndutils::all_zero_policy_uni>;

  explicit cached_rdist(size_t cache_size) :
    pivot_(cache_size),
    buf_(cache_size)
  {}

  template <typename RaIt>
  void mutate(RaIt first, RaIt last) {
    rdist_.mutate(first, last);
    pivot_ = buf_.size();
  }

  int operator()() {
    if (pivot_ == buf_.size()) {
      fill();
      pivot_ = 0;
    }
    return buf_[pivot_++];
  }

private:
  void fill() {
    tbb::parallel_for(tbb::blocked_range<size_t>{ 0ull, buf_.size() }, [&](const auto range) {
      auto reng = rndutils::make_random_engine_low_entropy<>();
      for (auto i = range.begin(); i < range.end(); ++i) {
        buf_[i] = rdist_(reng);
      }
    });
  }

  size_t pivot_;
  rdist_t rdist_;
  std::vector<int> buf_;
};

