#include "walksat.h"
#include <algorithm>
#include <map>
#include <unordered_map>


#ifdef RAND_DEVICE
std::random_device walksat::rd;
std::mt19937 walksat::gen(rd());
std::uniform_int_distribution<> walksat::_binary_dis(0,1);
std::uniform_real_distribution<> walksat::_real_dis(0,1);
#else
// int walksat::rd_noise=0.567*RAND_MAX;
#endif

bool walksat::Clause::satisfied_by(const Model &model)const{
	return std::any_of(this->begin(),this->end(),[&](const auto &v){
		auto v0=std::abs(v);
		return (v>0&&model[v0])||(v<0&&model[v0]==0);
	});
}


bool walksat::SAT::satisfied_by(const Model &model)const{
	return std::all_of(cnf.begin(),cnf.end(),[&](const auto &c){
		return c.satisfied_by(model);
	});
}

walksat::Atom walksat::v1::flip_to_satisfy_max(const CNF &cnf, const Clause &from, const Model &current){
	std::vector<std::pair<Atom,uint>> cnt;
	for(auto &&v:from){
		cnt.emplace_back(std::abs(v),0);
	}

	auto beg=cnt.begin();
	auto end=cnt.end();

	for(auto &&cls:cnf){
		for(auto &&v:cls){
			auto v0=std::abs(v);

			if((v>0&&current[v0]==0)||(v<0&&current[v0])){
				auto iter=std::find_if(beg,end,[&](auto &t){
					return t.first==v0;
				});

				if(iter!=end){
					++iter->second;
				}
			}
		}
	}

	return std::max_element(beg,end,[](auto &kv1,auto &kv2){
		return (kv1.second)-(kv2.second);
	})->first;
}


walksat::Atom walksat::v0::flip_to_satisfy_max(const CNF &cnf, const Clause &from, const Model &current){
	// std::vector<uint> cnt(current.size(),0);

	std::map<Atom,uint> cnt;
	
	for(const auto &v:from){
		cnt[std::abs(v)]=0;
	}

	for(const auto &cls:cnf){
		for(const auto &v:cls){
			auto v0=std::abs(v);
			if(cnt.count(v0) && ((v>0&&current[v0]==0)||(v<0&&current[v0]))){
				++cnt[v0];
			}
		}
	}

	return std::max_element(cnt.begin(),cnt.end(),[](auto &kv1,auto &kv2){
		return (kv1.second)-(kv2.second);
	})->first;
}


walksat::Model walksat::v0::walksat(const SAT &problem, double p, int max_flips) {
    auto model=Model(problem.num_var+1);
	const auto &cnf=problem.cnf;
    for(auto &v:model){
        v=binary_dis();
    }

	for(auto i=0;i<max_flips;++i){
		std::vector<const Clause*> unsatisfied_cls;
		for(const auto &c:cnf){
			if(!c.satisfied_by(model)){
				unsatisfied_cls.push_back(&c);
			}
		}

		if(unsatisfied_cls.empty()){
			return model;
		}

		auto rand_cls_idx=rand_range(unsatisfied_cls.size());
		const auto &rand_cls=*(unsatisfied_cls[rand_cls_idx]);

		if(real_dis()<p){
			auto rand_atom_idx=rand_range(rand_cls.size());

			auto rand_atom=std::abs(rand_cls[rand_atom_idx]);

			model[rand_atom]=!model[rand_atom];
		}
		else{
			auto atom=v1::flip_to_satisfy_max(cnf,rand_cls,model);

			model[atom]=!model[atom];
		}

	}

	return {};
}


walksat::SAT walksat::read_file(std::string file_name){
	auto fp=fopen(file_name.c_str(),"r");

	if(!fp){
		fprintf(stderr, "File not exists\n");
		exit(-1);
	}

	while (true){
		char ch=fgetc(fp);
		if(ch=='c'){
			fscanf(fp,"%*[^\n]\n");
		}
		else{
			ungetc(ch,fp);
			break;
		}
	}

	uint num_atom,num_cls;
	if (fscanf(fp, "p cnf %u %u", &num_atom, &num_cls) != 2){
		fprintf(stderr, "Invalid input file\n");
		exit(-1);
	}

	// std::vector<uint> occur(num_atom+1,0);
	// uint occured=0;
	auto cnf=CNF(num_cls);
	for(auto i=0;i<num_cls;++i){
		while (true){
			int t;
			if(fscanf(fp,"%d",&t)!=1){
				fprintf(stderr, "Invalid input file\n");
				exit(-1);
			}

			if(t==0){
				break;
			}
			else{
				cnf[i].push_back(t);

				// if(occur[std::abs(t)]++==0){
				// 	++occured;
				// }
			}
		}
		
	}

	return {std::move(cnf),num_atom};

	// if(occured==num_atom){
	// 	return {SAT{std::move(cnf),num_atom},{}};
	// }

	// auto to_new_atoms=std::vector<Atom>(num_atom+1);
	// auto unused=std::vector<Atom>();
	// unused.reserve(num_atom-occured);

	// Atom prev=1;
	// for(auto v=1;v<=num_atom;++v){
	// 	if(occur[v]){
	// 		to_new_atoms[v]=prev++;
	// 	}
	// 	else{
	// 		unused.push_back(v);
	// 	}
	// }


	// for(auto &cls:cnf){
	// 	for(auto v:cls){
	// 		if(v>0){
	// 			v=to_new_atoms[v];
	// 		}
	// 		else{
	// 			v=-to_new_atoms[v];
	// 		}
	// 	}
	// }

	// return {SAT{std::move(cnf),occured},{unused}};
	
}


bool walksat::verify_model(const CNF &cnf,const Model &model){
	return std::all_of(cnf.begin(),cnf.end(),[&](auto &&c){
		return c.satisfied_by(model);
	});
}