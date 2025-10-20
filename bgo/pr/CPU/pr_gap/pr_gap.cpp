// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#include <algorithm>
#include <iostream>
#include <vector>

#include "builder.h"
#include "command_line.h"
#include "graph.h"
#include "pvector.h"

#include "pr_gap.hpp"

/*
GAP Benchmark Suite
Kernel: PageRank (PR)
Author: Scott Beamer

Will return pagerank scores for all vertices once total change < epsilon

This PR implementation uses the traditional iterative approach. It performs
updates in the pull direction to remove the need for atomics, and it allows
new values to be immediately visible (like Gauss-Seidel method). The prior PR
implementation is still available in src/pr_spmv.cc.
*/


using namespace std;

typedef float ScoreT;
const float kDamp = 0.85;

pvector<ScoreT> PageRankPullGS(const PRGraph &g, int max_iters, double epsilon=0) {
  const ScoreT init_score = 1.0f / g.num_nodes();
  const ScoreT base_score = (1.0f - kDamp) / g.num_nodes();
  pvector<ScoreT> scores(g.num_nodes(), init_score);
  pvector<ScoreT> outgoing_contrib(g.num_nodes());
  #pragma omp parallel for
  for (int32_t n=0; n < g.num_nodes(); n++)
    outgoing_contrib[n] = init_score / g.out_degree(n);
  for (int iter=0; iter < max_iters; iter++) {
    double error = 0;
    #pragma omp parallel for reduction(+ : error) schedule(dynamic, 16384)
    for (int32_t u=0; u < g.num_nodes(); u++) {
      ScoreT incoming_total = 0;
      for (int32_t v : g.in_neigh(u))
        incoming_total += outgoing_contrib[v];
      ScoreT old_score = scores[u];
      scores[u] = base_score + kDamp * incoming_total;
      error += fabs(scores[u] - old_score);
      outgoing_contrib[u] = scores[u] / g.out_degree(u);
    }
    if (error < epsilon)
      break;
  }
  return scores;
}

int PR_gap(const PRGraph &G, CArray<float> *PR) {
  pvector<ScoreT> scores = PageRankPullGS(G, 20, 1e-4);
  PR->init(scores.size());
  for (unsigned int n=0; n < scores.size(); n++)
    PR->data[n] = scores[n];
  return 0;
}