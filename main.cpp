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
#ifndef RAND_DEVICE
		fprintf(stderr, "Use: ./walksatSNC filename [seed] [number of atoms per line (for n-queens)]\n");
#else
		fprintf(stderr, "Use: ./walksatSNC filename [number of atoms per line (for n-queens)]\n");
#endif
		return 1;
	}

	auto sat = read_file(argv[1]);

#ifndef RAND_DEVICE
	if (argc > 2) {
		uint t;
		sscanf(argv[2], "%d", &t);
		set_seed(t);
	}

	int line_len = 0;

	if (argc > 3) {
		sscanf(argv[3], "%d", &line_len);
	}

#else
	int line_len = 0;

	if (argc > 2) {
		sscanf(argv[2], "%d", &line_len);
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

	auto t_start = chrono::steady_clock::now();

	auto solver = walksatSNC(sat.cnf, sat.num_var);

	bool found = false;
	uint flips = 0;
	int trial;

	for (trial = 0; trial < 10; ++trial) {

		tie(found, flips) = solver.solve(0.567, 1000000);

		if (found) {
			break;
		}
	}

	auto t_end = std::chrono::steady_clock::now();

	auto elapsed_seconds = chrono::duration<double>(t_end - t_start);

	printf("Running time: %.4fs\n", elapsed_seconds.count());

	if (found) {
		auto &&mod = solver.current;
		printf("Solution:\n");

		if (line_len) {
			for (auto i = 1; i < mod.size(); ++i) {
				printf("%c", mod[i] ? '_' : 'X');
				
				if (i % line_len == 0) {
					putchar('\n');
				} else {
					putchar(' ');
				}
			}
		} else {
			for (auto i = 1; i < mod.size(); ++i) {
				printf("%d ", mod[i] ? i : -i);
			}
		}

		printf("\n\n");

		auto t = verify_model(sat.cnf, mod);
		printf("try: %d, flip: %d, verified: %d\n", trial, flips, t);
		return !t;
	} else {
		printf("Not found!\n");
		return 1;
	}
}