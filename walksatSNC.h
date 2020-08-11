#ifndef A5_WALKSATSNC_H
#define A5_WALKSATSNC_H
#include "walksat.h"
#include <array>

namespace walksat {
struct walksatSNC {
	const CNF &cnf;
	uint num_var;
	uint num_cls;
	// std::vector<unsigned char> NT_updated;
	std::vector<uint> NT;
	Model current;
	std::vector<uint> unsat_cls;
	std::vector<uint> unsat_cls_idx;

	std::vector<std::array<std::vector<uint>, 2>> _TLC;
	std::vector<uint> _clause_order;
	std::vector<const uint *> _var_iters;

	// uint NT(uint clause_idx);
	const std::vector<uint> &TLC(Atom v);

	walksatSNC(const CNF &cnf, uint num_var);

	Atom pickVar(const Clause &c, double p);
	void flip(Atom a);
	void _init_rand();
	std::pair<bool,uint> solve(double p = 0.567, int max_flip = 1000);
};

// Model walksat(const SAT &problem, double p=0.567, int max_flips=1000);

} // namespace walksat

#endif