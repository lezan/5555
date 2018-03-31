// TI2.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

struct node {
	char key;
	node* father;
	node* sibling;
	node* firstChildren;
	int counter;
	double frequency;
	double entropy;
	int level;
};

typedef node* dictionary;

struct IntCmp {
	bool operator()(const pair<char, double> &lhs, const pair<char, double> &rhs) {
		return lhs.second > rhs.second;
	}
};

double log_2(double n) {
	return log(n) / log(2.0);
}

dictionary root() {
	dictionary value = new node;
	value->key = '\0';
	value->father = NULL;
	value->firstChildren = NULL;
	value->sibling = NULL;
	value->counter = 0;
	value->frequency = 0.0;
	value->entropy = 0.0;
	value->level = 0;
	return value;
}

dictionary newNode(node* father) {
	dictionary value = new node;
	value->key = '\0';
	value->father = father;
	value->firstChildren = NULL;
	value->sibling = NULL;
	value->counter = 0;
	value->frequency = 0.0;
	value->entropy = 0.0;
	value->level = 0;
	return value;
}

void addWord(dictionary &d, string word) {
	d->counter++;
	int size = word.size();
	int counter = 0;
	int level = 0;

	if (d->firstChildren == NULL) {
		d->firstChildren = newNode(d);
		d->firstChildren->key = (word.c_str())[counter];
		d->firstChildren->counter++;
		--size;
	}
	d->firstChildren->level = ++level;

	dictionary iterator = d->firstChildren;

	while (size) {
		if (iterator->key != (word.c_str())[counter]) {
			if (iterator->sibling != NULL) {
				iterator = iterator->sibling;
			}
			else {
				iterator->sibling = newNode(iterator->father);
				iterator->sibling->key = (word.c_str())[counter];
				iterator->sibling->level = iterator->level;
				iterator = iterator->sibling;
				iterator->counter++;
				--size;
			}
		}
		else {
			if (iterator->firstChildren != NULL) {
				counter++;
				iterator->counter++;
				iterator = iterator->firstChildren;
				++level;
			}
			else {
				iterator->firstChildren = newNode(iterator); 
				iterator->firstChildren->key = (word.c_str())[++counter];
				iterator->firstChildren->level = ++level;
				iterator = iterator->firstChildren;
				iterator->counter++;
			}
			--size;
		}
	}
}

double entropyOfFirstLevel(dictionary d) {
	map<char, int> occurrence;
	dictionary iterator = d->firstChildren;
	int nodeFirstLevel = 0;

	while (iterator != NULL) {
		occurrence[iterator->key] = iterator->counter;
		nodeFirstLevel += iterator->counter;
		iterator = iterator->sibling;
	}

	map<char, double> probability;
	for (map<char, int>::iterator it = occurrence.begin(); it != occurrence.end(); ++it) {
		probability[it->first] = (double)it->second / nodeFirstLevel;
	}

	double entropy = 0.0;
	for (map<char, double>::iterator it = probability.begin(); it != probability.end(); ++it) {
		entropy += it->second * log_2(it->second);
	}

	entropy = -entropy;

	return entropy;
}

double conditionalEntropy(dictionary d, int level) {

	dictionary iterator = d;
	vector<dictionary> pointers;
	double entropy = 0.0;

	while (iterator != NULL) {
		if (iterator->sibling != NULL) {
			pointers.push_back(iterator->sibling);
		}
		if (iterator->level == level) {
			vector<double> probabilityLevelConditioning; // Condizionante
			vector<double> probabilityLevelConditioned; // Condizionata

			probabilityLevelConditioning.push_back((double)iterator->counter / d->counter);
			iterator = iterator->firstChildren;
			while (iterator != NULL) {
				probabilityLevelConditioned.push_back((double)iterator->counter / iterator->father->counter);
				iterator = iterator->sibling;
			}
			double localEntropy = 0.0;
			for (vector<double>::iterator it = probabilityLevelConditioned.begin(); it != probabilityLevelConditioned.end(); ++it) {
				localEntropy += *it * log_2(*it);
			}
			entropy += probabilityLevelConditioning.back() * localEntropy;
			if (pointers.empty()) {
				iterator = NULL;
			}
			else {
				iterator = pointers.back();
				pointers.pop_back();
			}
		}
		else {
			iterator = iterator->firstChildren;
		}
	}
	
	entropy = -entropy;
	return entropy;
}

vector<double> conditionalEntropyAllLevels(dictionary d, int maxLength) {
	dictionary iterator = d;
	iterator = iterator->firstChildren;
	vector<dictionary> pointers;
	vector<double> entropy(maxLength + 1);
	entropy[0] = 0;

	while (iterator != NULL) {
		if (iterator->sibling != NULL) {
			pointers.push_back(iterator->sibling);
		}
		double probability = (double)iterator->counter / iterator->father->counter;
		double entropyLocal = probability * log_2(probability);
		entropy[iterator->level] += entropyLocal * ((double)iterator->father->counter / d->counter);
		iterator = iterator->firstChildren;
		if (iterator == NULL) {
			if (!pointers.empty()) {
				iterator = pointers.back();
				pointers.pop_back();
			}
		}
	}

	for (vector<double>::iterator it = entropy.begin(); it != entropy.end(); ++it) {
		entropy[it - entropy.begin()] = -entropy[it - entropy.begin()];
	}
	
	return entropy;
}

map<char, double> nextProbabilityNode(dictionary &pointer, char letter) {
	dictionary iterator = pointer;
	dictionary init = pointer;
	map<char, int> occurrence;
	while (iterator != NULL) {
		if (iterator->key != letter) {
			iterator = iterator->sibling;
		}
		else {
			init = iterator;
			iterator = iterator->firstChildren;
			break;
		}
	}

	pointer = iterator;

	while (iterator != NULL) {
		occurrence.insert(pair<char, int>(iterator->key, iterator->counter));
		iterator = iterator->sibling;
	}

	map<char, double> probability;
	for (map<char, int>::iterator it = occurrence.begin(); it != occurrence.end(); ++it) {
		probability.insert(pair<char, double>(it->first, (double)it->second / init->counter));
	}

	return probability;
}

double entropyOfDictionary(dictionary d, int maxLength) {


	dictionary iterator = d;
	iterator = iterator->firstChildren;
	vector<dictionary> pointers;
	double entropy = 0.0;

	while (iterator != NULL) {
		if (iterator->sibling != NULL) {
			pointers.push_back(iterator->sibling);
		}
		double probability = (double)iterator->counter / iterator->father->counter;
		double entropyLocal = probability * log_2(probability);
		entropy += entropyLocal * ((double)iterator->father->counter / d->counter);
		iterator = iterator->firstChildren;
		if (iterator == NULL) {
			if (!pointers.empty()) {
				iterator = pointers.back();
				pointers.pop_back();
			}
		}
	}

	entropy = -entropy;

	return entropy;
}

int main() {

#if 1
	
	/*
	
	INIZIO PARTE UNO

	*/

	ifstream source("354984si.ngl");
	dictionary d = root();
	string input;
	int counterWords = 0;
	int maxLength = 0;
	cout << "Computing... ";
	while (getline(source, input)) {
		if (input.length() > maxLength) {
			maxLength = input.length();
		}
		addWord(d, input);
		++counterWords;
	}

	cout << "Done." << endl << endl;

	ofstream log("log.txt", ios::binary);

	/*

	FINE PARTE UNO

	*/

#endif

#if 1

	/*

	INIZIO PARTE DUE

	*/

	double entropy = entropyOfFirstLevel(d);
	cout << "Entropia primo livello: " << entropy << endl << endl;
	log << "Entropia primo livello: " << entropy << endl << endl;

	int levelCondition = 2;

	double entropyConditional = conditionalEntropy(d, levelCondition);
	cout << "Entropia livello " << levelCondition + 1 << " e': " << entropyConditional << endl << endl;
	log << "Entropia livello " << levelCondition + 1 << " e': " << entropyConditional << endl << endl;;

	ofstream fileEntropyAllLeves("entropyAllLeves.txt", ios::binary);
	cout << "Entropia su tutti i livelli..." << endl;
	vector<double> entropyConditionalAllLevels = conditionalEntropyAllLevels(d, maxLength);
	for (vector<double>::iterator it = entropyConditionalAllLevels.begin(); it != entropyConditionalAllLevels.end(); ++it) {
		cout << "Livello " << it - entropyConditionalAllLevels.begin() << " : " << *it << endl;
		fileEntropyAllLeves << it - entropyConditionalAllLevels.begin() << "," << *it << endl;
	}

	/*

	FINE PARTE DUE

	*/

#endif

#if 0

	/*

	INIZIO PARTE TRE

	*/

	dictionary pointer = d->firstChildren;
	char character;
	while (cin.good()) {
		cout << endl << "Inserisci una lettera..." << endl;
		cin >> character;
		map<char, double> entropyLevel = nextProbabilityNode(pointer, character);
		vector<pair<char, double>> orderEntropyLevel(entropyLevel.begin(), entropyLevel.end());
		sort(orderEntropyLevel.begin(), orderEntropyLevel.end(), IntCmp());
		for (vector<pair<char, double>>::iterator it = orderEntropyLevel.begin(); it != orderEntropyLevel.end(); ++it) {
			cout << "Lettera " << it->first << " con probabilita': " << it->second << endl;
		}
	}

	/*

	FINE PARTE TRE

	*/

#endif

#if 1

	/*

	INIZIO PARTE QUATTRO

	*/

	double entropyDictionary = entropyOfDictionary(d, maxLength);
	cout << endl << "Entropia del dizionario: " << entropyDictionary << endl << endl;
	log << endl << "Entropia del dizionario: " << entropyDictionary << endl << endl;

	double lengthCoding = log_2(counterWords);
	cout << endl << "Costo di una codifica a lunghezza fissa: " << lengthCoding << endl;
	log << endl << "Costo di una codifica a lunghezza fissa: " << lengthCoding << endl;

	/*

	FINE PARTE QUATTRO

	*/

#endif

	getchar();
	return 0;
}