// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#include <iostream>
#include <vector>

#include "bitmap.h"
#include "builder.h"
#include "command_line.h"
#include "graph.h"
#include "platform_atomics.h"
#include "pvector.h"
#include "sliding_queue.h"
#include "timer.h"

#include "bfs_gap.hpp"


/*
GAP Benchmark Suite
Kernel: Breadth-First Search (BFS)
Author: Scott Beamer

Will return parent array for a BFS traversal from a source vertex

This BFS implementation makes use of the Direction-Optimizing approach [1].
It uses the alpha and beta parameters to determine whether to switch search
directions. For representing the frontier, it uses a SlidingQueue for the
top-down approach and a Bitmap for the bottom-up approach. To reduce
false-sharing for the top-down approach, thread-local QueueBuffer's are used.

To save time computing the number of edges exiting the frontier, this
implementation precomputes the degrees in bulk at the beginning by storing
them in the parent array as negative numbers. Thus, the encoding of parent is:
  parent[x] < 0 implies x is unvisited and parent[x] = -out_degree(x)
  parent[x] >= 0 implies x been visited

[1] Scott Beamer, Krste Asanović, and David Patterson. "Direction-Optimizing
	Breadth-First Search." International Conference on High Performance
	Computing, Networking, Storage and Analysis (SC), Salt Lake City, Utah,
	November 2012.
*/


using namespace std;

int64_t BUStep(const BFSGraph &g, pvector<int32_t> &parent, Bitmap &front,
			   Bitmap &next) {
	int64_t awake_count = 0;
	next.reset();
	#pragma omp parallel for reduction(+ : awake_count) schedule(dynamic, 1024)
	for (int32_t u=0; u < g.num_nodes(); u++) {
		if (parent[u] < 0) {
			for (int32_t v : g.in_neigh(u)) {
				if (front.get_bit(v)) {
					parent[u] = v;
					awake_count++;
					next.set_bit(u);
					break;
				}
			}
		}
	}
	return awake_count;
}


int64_t TDStep(const BFSGraph &g, pvector<int32_t> &parent,
			   SlidingQueue<int32_t> &queue) {
	int64_t scout_count = 0;
	#pragma omp parallel
	{
		QueueBuffer<int32_t> lqueue(queue);
		#pragma omp for reduction(+ : scout_count) nowait
		for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
			int32_t u = *q_iter;
			for (int32_t v : g.out_neigh(u)) {
				int32_t curr_val = parent[v];
				if (curr_val < 0) {
					if (compare_and_swap(parent[v], curr_val, u)) {
						lqueue.push_back(v);
						scout_count += -curr_val;
					}
				}
			}
		}
		lqueue.flush();
	}
	return scout_count;
}


void QueueToBitmap(const SlidingQueue<int32_t> &queue, Bitmap &bm) {
	#pragma omp parallel for
	for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
		int32_t u = *q_iter;
		bm.set_bit_atomic(u);
	}
}

void BitmapToQueue(const BFSGraph &g, const Bitmap &bm,
				   SlidingQueue<int32_t> &queue) {
	#pragma omp parallel
	{
		QueueBuffer<int32_t> lqueue(queue);
		#pragma omp for nowait
		for (int32_t n=0; n < g.num_nodes(); n++)
			if (bm.get_bit(n))
				lqueue.push_back(n);
		lqueue.flush();
	}
	queue.slide_window();
}

pvector<int32_t> InitParent(const BFSGraph &g) {
	pvector<int32_t> parent(g.num_nodes());
	#pragma omp parallel for
	for (int32_t n=0; n < g.num_nodes(); n++)
		parent[n] = g.out_degree(n) != 0 ? -g.out_degree(n) : -1;
	return parent;
}

pvector<int32_t> DOBFS(const BFSGraph &g, int32_t source, int alpha = 15, int beta = 18) {
	pvector<int32_t> parent = InitParent(g);
	parent[source] = source;
	SlidingQueue<int32_t> queue(g.num_nodes());
	queue.push_back(source);
	queue.slide_window();
	Bitmap curr(g.num_nodes());
	curr.reset();
	Bitmap front(g.num_nodes());
	front.reset();
	int64_t edges_to_check = g.num_edges_directed();
	int64_t scout_count = g.out_degree(source);
	while (!queue.empty()) {
		if (scout_count > edges_to_check / alpha) {
			int64_t awake_count, old_awake_count;
			QueueToBitmap(queue, front);
			awake_count = queue.size();
			queue.slide_window();
			do {
				old_awake_count = awake_count;
				awake_count = BUStep(g, parent, front, curr);
				front.swap(curr);
			} while ((awake_count >= old_awake_count) ||
			(awake_count > g.num_nodes() / beta));
			BitmapToQueue(g, front, queue);
			scout_count = 1;
		} else {
			edges_to_check -= scout_count;
			scout_count = TDStep(g, parent, queue);
			queue.slide_window();
		}
	}
	#pragma omp parallel for
	for (int32_t n = 0; n < g.num_nodes(); n++)
		if (parent[n] < -1)
			parent[n] = -1;
	return parent;
}

/* Helper function that calls GAP BC, taking parameters as input that are compatible with autobench. */
int bfs_gap(BFSGraph &G, int source, CArray<int> *level, CArray<int> *parent) {
	pvector<int32_t> result = DOBFS(G, source);
	parent->init(result.size());
	for (unsigned int n=0; n < result.size(); n++)
		parent->data[n] = result[n];
	return 0;
}
