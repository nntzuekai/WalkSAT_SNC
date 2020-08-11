#include "walksatSNC.h"
#include <algorithm>

// uint walksatSNC::NT(uint clause_idx){
// 	if(NT_updated[clause_idx]){
// 		return _NT[clause_idx];
// 	}
// 	else{
// 		uint cnt=0;
// 		for(auto &&v:cnf[clause_idx]){
// 			Atom v0=std::abs(v);
// 			if((v>0&&current[v0]>0)||(v<0&&current[v0]==0)){
// 				++cnt;
// 			}
// 		}

// 		_NT[clause_idx]=cnt;
// 		NT_updated[clause_idx]=true;

// 		return cnt;
// 	}
// }

walksat::walksatSNC::walksatSNC(const CNF &cnf, uint num_var)
    : cnf(cnf), num_var(num_var), num_cls(cnf.size()), NT(num_cls),
      current(num_var + 1), unsat_cls_idx(num_cls), _TLC(num_var + 1) {

#ifndef RAND_DEVICE
	auto t=time(nullptr);
	srand(t);
#ifndef NDEBUG
	printf("seed: %ld\n",t);
#endif	
#endif

	unsat_cls.reserve(num_cls);
	for (auto i = 0; i < num_cls; ++i) {
		for (auto &&v : cnf[i]) {
			auto v0 = std::abs(v);
			if (v < 0) {
				_TLC[v0][0].push_back(i);
			} else {
				_TLC[v0][1].push_back(i);
			}
		}
	}
}

const std::vector<uint> &walksat::walksatSNC::TLC(Atom v) {
	auto v0 = std::abs(v);
	return _TLC[v0][(v > 0) ? 1 : 0];
}

void walksat::walksatSNC::_init_rand(){
	for (auto i = 1; i <= num_var; ++i) {
		current[i] = binary_dis();
	}
	
	std::fill(NT.begin(),NT.end(),0);

	unsat_cls.clear();

	for (auto i = 0; i < num_cls; ++i) {
		for (auto &&v : cnf[i]) {
			auto v0 = std::abs(v);
			if ((v > 0 && current[v0] == 1) || (v < 0 && current[v0] == 0)) {
				++NT[i];
			}
		}

		if (NT[i] == 0) {
			unsat_cls.push_back(i);
			unsat_cls_idx[i] = unsat_cls.size() - 1;
		}
	}
}

std::pair<bool,uint> walksat::walksatSNC::solve(double p, int max_flip) {
	_init_rand();

#ifndef NDEBUG
	printf("try: \n");
	for(auto v:current){
		printf("%d ", v);
	}
	putchar('\n');

#endif

	for(auto k=0;k<max_flip;++k){
		if(unsat_cls.empty()){
			return {true,k};
		}

		// uint c_idx=unsat_cls[rand_range(0,unsat_cls.size()-1)];
		uint c_idx=unsat_cls[rand_range(unsat_cls.size())];

		auto &&c=cnf[c_idx];
		auto a=pickVar(c,p);
		flip(a);
	}

	return {false,max_flip};
}

void walksat::walksatSNC::flip(Atom a){
#ifndef NDEBUG
	printf("flip: %d\n",a);

#endif

	current[a]=!current[a];
	auto v0=std::abs(a);

	auto &&tlc_0=_TLC[v0][current[a]];
	for(auto c_idx:tlc_0){
		++NT[c_idx];
		if(NT[c_idx]==1){
			
			uint unsat_idx=unsat_cls_idx[c_idx];
			if(unsat_idx==unsat_cls.size()-1){
				unsat_cls.pop_back();
			}
			else{
				uint back_c_idx=unsat_cls.back();
				unsat_cls[unsat_idx]=back_c_idx;
				unsat_cls.pop_back();

				unsat_cls_idx[back_c_idx]=unsat_idx;
			}
		}
	}

	auto &&tlc_1=_TLC[v0][!current[a]];
	for(auto c_idx:tlc_1){
		--NT[c_idx];
		if(NT[c_idx]==0){
			unsat_cls.push_back(c_idx);
			unsat_cls_idx[c_idx]=unsat_cls.size()-1;
		}
	}

	// current[a]=!cur_v;
}

walksat::Atom walksat::walksatSNC::pickVar(const Clause &c,double p){
	uint num_lit=c.size();

	_clause_order.clear();
	_clause_order.resize(num_lit);
	for(auto i=0;i<num_lit;++i){
		_clause_order[i]=i;
	}

#ifndef RAND_DEVICE
	std::random_shuffle(_clause_order.begin(),_clause_order.end());
#else
	std::shuffle(_clause_order.begin(),_clause_order.end(),gen);
#endif

	_var_iters.resize(num_lit);

	for(auto i=0;i<num_lit;++i){
		// auto zero_break=true;
		auto v=c[_clause_order[i]];
		auto v0=std::abs(v);

		auto &&tlc=_TLC[v0][current[v0]];
		const uint *p=tlc.data();
		auto *end=p+tlc.size();

		for(;p<end;++p){
			if(NT[*p]==1){
				// zero_break=false;
				break;
			}
		}

		if(p>=end){
#ifndef NDEBUG
			printf("0-break\n");

#endif
			return v0;
		}

		_var_iters[i]=p;
	}


	if(real_dis()<p){
#ifndef NDEBUG
		printf("random\n");

#endif
		return std::abs(c[_clause_order[0]]);
	}
	else{
#ifndef NDEBUG
		printf("bsetVar\n");

#endif
		Atom bestVar;
		uint breakBestVar=0;

		{
			auto v=c[_clause_order[0]];
			auto v0=std::abs(v);
			bestVar=v0;
			auto &&tlc0=_TLC[v0][current[v0]];
			auto *end=tlc0.data()+tlc0.size();
			for(auto p=_var_iters[0]+1;p<end;++p){
				if(NT[*p]==1){
					++breakBestVar;
				}
			}

			if(breakBestVar==0){
				return v0;
			}
		}

		for(auto i=1;i<num_lit;++i){
			uint break_v=0;

			auto v=c[_clause_order[i]];
			auto v0=std::abs(v);
			auto &&tlc=_TLC[v0][current[v0]];

			auto *end=tlc.data()+tlc.size();

			auto p=_var_iters[i]+1;
			for(;p<end;++p){
				if(NT[*p]==1){
					if(break_v==breakBestVar-1){
						break;
					}
					++break_v;
				}
			}

			if(p>=end){
				if(break_v==0){
					return v0;
				}
				else{
					bestVar=v0;
					breakBestVar=break_v;
				}
			}

		}
		return bestVar;
	}
}

// walksat::Model walksat::SNC::walksat(const SAT &problem, double p,
//                                      int max_flips) {
// 	auto model = Model(problem.num_var + 1);
// 	const auto &cnf = problem.cnf;
	
// 	for (auto &v : model) {
// 		v = _binary_dis(gen);
// 	}

// 	std::vector<const Clause *> unsatisfied_cls;
// 	for (auto i = 0; i < max_flips; ++i) {
// 		unsatisfied_cls.clear();

// 		for (const auto &c : cnf) {
// 			if (!c.satisfied_by(model)) {
// 				unsatisfied_cls.push_back(&c);
// 			}
// 		}

// 		if (unsatisfied_cls.empty()) {
// 			return model;
// 		}

// 		auto rand_cls_idx =
// 		    std::uniform_int_distribution<>{0, unsatisfied_cls.size() - 1}(gen);
// 		const auto &rand_cls = *(unsatisfied_cls[rand_cls_idx]);
// 		if (_real_dis(gen) < p) {
// 			auto rand_atom_idx =
// 			    std::uniform_int_distribution<>{0, rand_cls.size() - 1}(gen);
// 			auto rand_atom = std::abs(rand_cls[rand_atom_idx]);

// 			model[rand_atom] = !model[rand_atom];
// 		} else {
// 			auto atom = v1::flip_to_satisfy_max(cnf, rand_cls, model);

// 			model[atom] = !model[atom];
// 		}
// 	}

// 	return {};
// }
