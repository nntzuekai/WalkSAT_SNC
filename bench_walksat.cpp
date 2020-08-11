#include "walksat.h"
#include "walksatSNC.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>

using namespace std;
using namespace walksat;
int main(int argc, char **argv) {

	if (argc < 2) {
#ifdef RAND_DEVICE
		fprintf(stderr, "Use: ./walksatSNC filename [iter=10]\n");
#else	
		fprintf(stderr, "Use: ./walksatSNC filename [iter=10] [seed]\n");
#endif
		return 1;
	}

	auto sat = read_file(argv[1]);
	int iter = 10;

	if (argc > 2) {
		sscanf(argv[2], "%d", &iter);
	}

#ifndef RAND_DEVICE
	if (argc > 3) {
		uint t;
		sscanf(argv[3], "%d", &t);
		set_seed(t);
	}
#endif
	// auto
	// [sat,unused]=read_file("/home/jerry/Workspace/Walksat_v56/queens/8.cnf");
	// auto
	// [sat,unused]=read_file("/home/jerry/Workspace/Walksat_v56/Wffs/f200-850-v3.cnf");

	// printf("%lu\n",unused.unused_atoms.size());

	// auto p1=SAT(CNF{Clause({1,-3}),Clause({2,3,-1})},3);
	// auto sat = SAT(CNF{{1,-3}, {3,-1},{2},{3}}, 3);

	// auto sat=SAT(CNF{{1,-2,-3,-4},{-1,2,-3,-4},{-1,-2,3,-4},{-1,-2,-3,4}},4);

	bool found = false;
	uint flips = 0;
	uint total_flips=0;

	uint success=0;

	auto t_start = chrono::steady_clock::now();

	auto solver = walksatSNC(sat.cnf, sat.num_var);
	for (auto trial = 0; trial < iter; ++trial) {

		tie(found, flips) = solver.solve(0.567, 1000000);

		if (found) {
			++success;
		}
		total_flips+=flips;
	}

	auto t_end = std::chrono::steady_clock::now();

	auto elapsed_seconds = chrono::duration<double>(t_end - t_start);

	printf("Run %d times. average running time: %.6fs\n",iter, elapsed_seconds.count());
	printf("Succeeded for %d times (%f %%)\n",success,100.0*success/iter);
	printf("total flips: %u\n", total_flips);
	
}