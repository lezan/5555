#include "random.h"
#include <vector>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <random>
#include <chrono>

#define ESP1 0
#define ESP2 1
#define ESP3 0
#define VERBOSE 0
#define LOG 1
using namespace std;

struct benchmark {
	uint32_t k;
	uint32_t n;
	uint32_t numberTests = 0;
	uint32_t successfulTests = 0;	
};

vector<uint32_t> initSource(uint32_t k) {
	srand(time(0));
	vector<uint32_t> source;
	for (uint32_t i = 0; i < k; ++i) {
		source.push_back(rand());
	}
	return source;
}

vector< vector<uint32_t> > sortRelationship(vector< vector<uint32_t> > relationship) {
	for (vector< vector<uint32_t> >::iterator iterator = relationship.begin(); iterator != relationship.end(); ++iterator) {
		sort(iterator->begin(), iterator->end());
	}
	return relationship;
}

bool coinFlip() {
	initializer_list<double> probabilities = { 0.5, 0.5 };
	random_device rd;
	mt19937 gen(rd());
	discrete_distribution<int> distribution(probabilities);
	uint32_t symbol = distribution(gen);
	if (symbol == 0) {
		return true;
	}
	return false;
}

benchmark run(uint32_t k = 100, uint32_t n = 150, double c = 0.01, double delta = 0.01, uint32_t tests = 1, string distribution = "rsd") {

	benchmark result = {};
	result.k = k;
	result.n = n;
	result.numberTests = tests;

	Random rndgen;
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	default_random_engine uniformGenerator(seed);
	uniform_int_distribution<uint32_t> uniformDistribution(0, k - 1);
	if (distribution == "rsd") {
		rndgen.init_rsd(k, delta, c);
	}
	vector<uint32_t> source = initSource(k);

#if VERBOSE
	cout << "c: " << c << "\t" << "delta: " << delta << endl;
#endif

	/*

	CODIFICO

	*/

	for (uint32_t test = 0; test < tests; ++test) {

		vector<uint32_t> output;
		vector< vector<uint32_t> > relationship(n);

		vector<uint32_t> decoded(k);

		uint32_t counterDecodedSymbols = 0;

		uint32_t it = 0;
		uint32_t jt;

		bool found = false;

		for (uint32_t i = 0; i < n; ++i) {
			
			uint32_t coded = 0;

			if (distribution == "rsd" || distribution == "uniform") {
				uint32_t random = 0;
				if (distribution == "rsd") {
					random = rndgen.rsd();
				}
				else {
					random = uniformDistribution(uniformGenerator);
				}

				if (random != 100) {
					for (uint32_t y = 0; y < random; ++y) {
						uint32_t randomIndexSource = rand() % k;
						while (true) {
							vector<uint32_t>::iterator it1 = find(relationship[i].begin(), relationship[i].end(), randomIndexSource);
							if (it1 != relationship[i].end()) {
								randomIndexSource = rand() % k;
							}
							else {
								break;
							}
						}
						relationship[i].push_back(randomIndexSource);
						coded ^= source[randomIndexSource];
					}
				}
				else {
					for (uint32_t y = 0; y < k; ++y) {
						relationship[i].push_back(y);
						coded ^= source[y];
					}
				}
			}
			else {
				for (uint32_t y = 0; y < k; ++y) {
					if (coinFlip()) {
						relationship[i].push_back(y);
						coded ^= source[y];
					}
				}
			}
			output.push_back(coded);
		}

		//relationship = sortRelationship(relationship);

		/*

		DECODIFICO

		*/

		while (counterDecodedSymbols < k) {
			for (it = 0; it < relationship.size(); ++it) {
				if (relationship[it].size() == 1) {
					++counterDecodedSymbols;
					jt = relationship[it][0];
					decoded[jt] = output[it];
					found = true;
					break;
				}
			}

			if (found) {
				found = false;
			}
			else {
				break;
			}
			if (counterDecodedSymbols == k) {
				break;
			}

			for (uint32_t zt = 0; zt < relationship.size(); ++zt) {
				for (uint32_t kt = 0; kt < relationship[zt].size(); ++kt) {
					//if (relationship[zt][kt] <= jt) { // Funziona solo le il vector è ordinato
						if (relationship[zt][kt] == jt && zt != it) {
							if (relationship[zt].size() != 1) {
								output[zt] ^= decoded[jt];
								relationship[zt].erase(kt + relationship[zt].begin());
							}
							else {
								relationship.erase(zt + relationship.begin());
								output.erase(zt + output.begin());
							}
							break;
						}
					/*}
					else {
						break;
					}*/
				}
			}
			relationship.erase(it + relationship.begin());
			output.erase(it + output.begin());
		}
		if (counterDecodedSymbols == k) {
			++result.successfulTests;
		}
	}
	return result;
}

bool checkUnique(vector< vector<uint32_t> > relationship) {
	relationship = sortRelationship(relationship);
	//int counterNotUnique = 0;
	//vector<int> temp;
	for (vector< vector<uint32_t> >::iterator iterator = relationship.begin(); iterator != relationship.end(); ++iterator) {
		vector<uint32_t>::iterator yt = adjacent_find(iterator->begin(), iterator->end());
		if (yt != iterator->end()) {
			//++counterNotUnique;
			//temp.push_back(iterator - relationship.begin());
			return false;
		}
	}
	return true;
}

int main(int argc, char * argv[]) {
	double c = 0.05;
	double delta = 0.05;

#if ESP1
	vector<uint32_t> k1RSD;
	k1RSD.push_back(50);
	k1RSD.push_back(100);
	k1RSD.push_back(200);
	k1RSD.push_back(500);

	vector<uint32_t> k1Uniform;
	k1Uniform.push_back(50);
	k1Uniform.push_back(100);
	k1Uniform.push_back(200);
	k1Uniform.push_back(500);

	vector<benchmark> results1RSD(4);
	vector<benchmark> results1Uniform(4);
#if LOG
	ofstream log1("log1.txt");
	log1 << "RSD" << endl << endl;
#endif
	uint32_t numberTests1 = 100;
#if VERBOSE
	cout << "# tests: " << numberTests1 << endl << endl;
#endif
	for (uint32_t i = 0; i < 4; ++i) {
		results1RSD[i] = run(k1RSD[i], 2 * k1RSD[i], c, delta, numberTests1, "rsd");
#if VERBOSE
		cout << "k: " << k1RSD[i] << "\t" << "n: " << 2 * k1RSD[i] << "\t" << "p(n): " << (double)results1RSD[i].successfulTests / results1RSD[i].numberTests << endl << endl;
#endif
#if LOG
		log1 << "k: " << k1RSD[i] << "\t" << "n: " << 2 * k1RSD[i] << "\t" << "p(n): " << (double)results1RSD[i].successfulTests / results1RSD[i].numberTests << endl;
#endif
	}
#if LOG
	log1 << "UNIFORM" << endl << endl;
#endif
	for (uint32_t i = 0; i < 4; ++i) {
		results1Uniform[i] = run(k1Uniform[i], 2 * k1Uniform[i], c, delta, numberTests1, "uniform");
#if VERBOSE
		cout << "k: " << k1Uniform[i] << "\t" << "n: " << 2 * k1Uniform[i] << "\t" << "p(n): " << (double)results1Uniform[i].successfulTests / results1Uniform[i].numberTests << endl << endl;
#endif
#if LOG
		log1 << "k: " << k1Uniform[i] << "\t" << "n: " << 2 * k1Uniform[i] << "\t" << "p(n): " << (double)results1Uniform[i].successfulTests / results1Uniform[i].numberTests << endl;
#endif
	}
#if LOG
	log1.close();
#endif
#endif
#if ESP2
	vector<uint32_t> k2;
	k2.push_back(50);
	k2.push_back(100);
	k2.push_back(200);
	k2.push_back(500);

	uint32_t numberTests2 = 100;
#if VERBOSE
	cout << "# tests: " << numberTests2 << endl << endl;
#endif
#if LOG
	ofstream log2("log2.txt");
	ofstream log2CSV("log2.csv");
#endif
	vector<benchmark> results2(44);
	for (int i = 0; i < 4; i++) {
		for (double increment = 1.0; increment < 2.1; increment += 0.1) {
			results2[i] = run(k2[i], increment * k2[i], c, delta, numberTests2, "rsd");
#if VERBOSE
			cout << "k: " << k2[i] << "\t" << "n: " << increment * k2[i] << "\t" << "p(n): " << (double)results2[i].successfulTests / results2[i].numberTests << endl << endl;
#endif
#if LOG
			log2 << "k: " << k2[i] << "\t" << "n: " << increment * k2[i] << "\t" << "p(n): " << (double)results2[i].successfulTests / results2[i].numberTests << endl << endl;
			log2CSV << k2[i] << "," << increment * k2[i] << "," << (double)results2[i].successfulTests / results2[i].numberTests << endl;
#endif
		}
	}
#if LOG
	log2.close();
#endif
#endif
#if ESP3
	vector<uint32_t> k3;
	k3.push_back(50);
	k3.push_back(100);
	k3.push_back(200);
	k3.push_back(500);

	vector<benchmark> results(4);
#if LOG
	ofstream log3("log3.txt");
#endif
	uint32_t numberTests3 = 100;
#if VERBOSE
	cout << "# tests: " << numberTests3 << endl << endl;
#endif
	for (uint32_t i = 0; i < 4; ++i) {
		results[i] = run(k3[i], 2 * k3[i], c, delta, numberTests3, "coin");
#if VERBOSE
		cout << "k: " << k3[i] << "\t" << "n: " << 2 * k3[i] << "\t" << "p(n): " << (double)results[i].successfulTests / results[i].numberTests << endl << endl;
#endif
#if LOG
		log3 << "k: " << k3[i] << "\t" << "n: " << 2 * k3[i] << "\t" << "p(n): " << (double)results[i].successfulTests / results[i].numberTests << endl;
#endif
	}
#if LOG
	log3.close();
#endif
#endif
	cout << "done.";
	getchar();
	return 1;
}