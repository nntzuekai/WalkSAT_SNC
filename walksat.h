#ifndef A5_WALKSAT_H
#define A5_WALKSAT_H

#include <cstdlib>
#include <vector>
#include <random>
#include <initializer_list>
#include <string>
#include <tuple>

namespace walksat{
	using uint=unsigned int;

#ifdef RAND_DEVICE
    extern std::random_device rd;
    extern std::mt19937 gen;
	extern std::uniform_int_distribution<> _binary_dis;
	extern std::uniform_real_distribution<> _real_dis;
#else
	// extern int rd_noise;
#endif

#ifdef RAND_DEVICE
	inline int binary_dis(){
		return _binary_dis(gen);
	}

	inline double real_dis(){
		return _real_dis(gen);
	}

	inline int rand_range(int a, int b){
		return std::uniform_int_distribution<>{a, b}(gen);
	}

	inline int rand_range(int end){
		return std::uniform_int_distribution<>{0, end-1}(gen);
	}
#else
	inline int binary_dis(){
		// return _binary_dis(gen);
		return rand()%2;
	}

	inline double real_dis(){
		// return _real_dis(gen);
		return 1.0*rand()/RAND_MAX;
	}

	inline int rand_range(int end){
		return rand()%end;
	}

	inline void set_seed(uint seed){
		srand(seed);
	}

	// inline void set_noise(double noise){
	// 	rd_noise=noise*RAND_MAX;
	// }
#endif

	using Atom=int;
    using Model=std::vector<unsigned char>;
    struct Clause:public std::vector<Atom>{
		// template<class T>
		// Clause(T &&t):vector(std::forward<T>(t)){}

		// template<class... U>
		// Clause(U&&... u):vector(std::forward<U>(u)...){}

		Clause(std::initializer_list<Atom> l):vector(l){}
		Clause():vector(){}


		bool satisfied_by(const Model &model)const;
	};
    using CNF=std::vector<Clause>;
    struct SAT{
        uint num_var;
        CNF cnf;

		SAT(const CNF &cnf,uint num_var):num_var(num_var),cnf(cnf){}
		SAT(CNF &&cnf,uint num_var):num_var(num_var),cnf(std::move(cnf)){}

		bool satisfied_by(const Model &model)const;
    };
	

	namespace v0{
		Atom flip_to_satisfy_max(const CNF &cnf, const Clause &from, const Model &current);
		Model walksat(const SAT &problem, double p=0.5, int max_flips=1000);

	}

	namespace v1{
		Atom flip_to_satisfy_max(const CNF &cnf, const Clause &from, const Model &current);
	}

	
	struct unused_compression{
		std::vector<Atom> unused_atoms;
	};
	
	SAT read_file(std::string file_name);


	bool verify_model(const CNF &cnf,const Model &model);
}




#endif //A5_WALKSAT_H
