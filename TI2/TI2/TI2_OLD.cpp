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

double log_2(double n) {
	// log(n)/log(2) is log2.  
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
		
	}d->firstChildren->level = ++level;
	dictionary iterator = d->firstChildren;
	while (size - 1) {
		if (iterator->key != (word.c_str())[counter]) { // Se la lettera contenuta dal nodo corrente è diversa, vuol dire che devo spostarmi a destra, cioè con il fratello maggiore
			if (iterator->sibling != NULL) { // Controllo che esista il fratello maggiore e se esiste mi sposto di una posizione a destra
				iterator = iterator->sibling;
			}
			else { // Se il fratello maggiore non esiste, allora lo vado a creare
				iterator->sibling = newNode(iterator->father); // Creo il nuovo nodo fratello
				iterator->sibling->key = (word.c_str())[counter]; // Inserisco la lettera
				iterator->sibling->level = iterator->father->firstChildren->level;
				iterator = iterator->sibling; // Aggiorno la posizione dell'iteratore sul nodo appena creato
				iterator->counter++;
			}
		}
		else { // Se la lettera contenuta dal nodo corrente è uguale, allora devo scendere di livello, cioè spostarmi nel nodo figlio
			if (iterator->firstChildren != NULL) { // Controllo se esiste il nodo figlio del nodo corrente e se esiste, aggiorno la posizione dell'iteratore sul figlio del nodo corrente
				counter++;
				iterator->counter++;
				iterator = iterator->firstChildren;
				++level;
			}
			else { // Allora non esiste il nodo figlio del nodo corrente e ne creo uno e gli assegno la lettera
				iterator->firstChildren = newNode(iterator); // Creo il nuovo nodo che avrà come padre il nodo corrente
				iterator->firstChildren->key = (word.c_str())[++counter]; // Inserisco la lettera nel nuovo nodo appena creato
				iterator->firstChildren->level = ++level;
				iterator = iterator->firstChildren; // Aggiorno l'iteratore sul nodo appena inserito
				iterator->counter++;
			}
			size--;
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

	/*
	for (map<char, int>::iterator it = occurrence.begin(); it != occurrence.end(); ++it) {
	cout << it->first << " => " << it->second << endl;
	}
	*/

	map<char, double> probability;
	for (map<char, int>::iterator it = occurrence.begin(); it != occurrence.end(); ++it) {
		probability[it->first] = (double)it->second / nodeFirstLevel;
	}

	/*
	for (map<char, double>::iterator it = probability.begin(); it != probability.end(); ++it) {
	cout << "Carattere " << it->first << " con probabilita': " << it->second << endl;
	}
	*/

	double entropy = 0.0;
	for (map<char, double>::iterator it = probability.begin(); it != probability.end(); ++it) {
		entropy += it->second * log_2(it->second);
	}

	entropy = -entropy;

	return entropy;
}

// Devo calcolare - per esempio per la parola BOB - l'entropia di B livello 3 data la O.
// H ( B | O ) = P ( c_l = O ) * H ( B | O ).
// P ( c_l = O) = 1;
// H ( B | O ) = 0; perché data O l'unico carattere possibile è B.
// Corretto?
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
	multimap<int, double> probability; // livello | probabilità

	while (iterator != NULL) {
		if (iterator->sibling != NULL) {
			pointers.push_back(iterator->sibling);
		}
		probability.insert(pair<int, double>(iterator->level, (double)iterator->counter / d->counter));
		iterator = iterator->firstChildren;
		if (iterator == NULL) {
			if (!pointers.empty()) {
				iterator = pointers.back();
				pointers.pop_back();
			}
		}
	}

	for (int i = 1; i <= maxLength; ++i) {
		for (multimap<int, double>::iterator it = probability.begin(); it != probability.end(); ++it) {
			if (it->first == i) {
				entropy[i] += it->second * log_2(it->second);
			}
		}
	}

	for (vector<double>::iterator it = entropy.begin(); it != entropy.end(); ++it) {
		entropy[it - entropy.begin()] = -entropy[it - entropy.begin()];
	}

	return entropy;
}

/*
double conditionalEntropyGivenCharacter(dictionary d, string word, int level) {

	int counter = 1;

	dictionary pointTo = d->firstChildren;
	dictionary iterator = d->firstChildren;

	while (level) {
		if (iterator->key == (word.c_str())[counter - 1]) {
			iterator = iterator->firstChildren;
			counter++;
			level--;
		}
		else {
			iterator = iterator->sibling;
		}
	}

	if (iterator->father->counter == 1) {
		return 0;
	}

	double probabilityCondition = 0.0;
	probabilityCondition = (double)iterator->father->counter / d->counter; // Probabilita' del carattere al livello l, ovvero la probabilità della lettera condizionante.

	int counterOccurrenceTotalChildrens = 0;
	int counterChildrens = 0;
	vector<int> occurrenceChildrens;
	while (iterator != NULL) {
		occurrenceChildrens.push_back(iterator->counter);
		counterChildrens++;
		counterOccurrenceTotalChildrens += iterator->counter;
		iterator = iterator->sibling;
	}

	vector<double> probabilityChildrens(counterChildrens);
	for (vector<int>::iterator it = occurrenceChildrens.begin(); it != occurrenceChildrens.end(); ++it) {
		probabilityChildrens[it - occurrenceChildrens.begin()] = (double)occurrenceChildrens[it - occurrenceChildrens.begin()] / counterOccurrenceTotalChildrens;
	}

	double entropyLevelChildrens = 0.0;
	for (vector<double>::iterator it = probabilityChildrens.begin(); it != probabilityChildrens.end(); ++it) {
		entropyLevelChildrens += probabilityChildrens[it - probabilityChildrens.begin()] * log_2(probabilityChildrens[it - probabilityChildrens.begin()]);
	}

	entropyLevelChildrens = -entropyLevelChildrens;

	return probabilityCondition * entropyLevelChildrens;
}
*/

map<char, double> nextProbabilityNode(dictionary &pointer, char letter) {
	dictionary iterator = pointer;
	dictionary init = pointer;
	map<char, int> occurrence;
	while (iterator != NULL) {
		if (iterator->key != letter) {
			iterator = iterator->sibling;
		}
		else {
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
		probability[it->first] = (double)it->second / init->counter;
	}

	return probability;
}

double entropyOfDictionary(dictionary d, int counterWords, int maxLength) {
	dictionary iterator = d;
	vector<dictionary> pointers;

	double entropy = 0.0;

	//int counter = 0;

	int counting = counterWords;

	multimap<int, double> probability; // first = livello, second = probabilità
	probability.insert(pair<int, double>(/*counter*/iterator->level, (double)iterator->counter));
	//counter++;
	iterator = d->firstChildren;

	while (counting) {
		while (iterator != NULL) {
			if (iterator->sibling != NULL) {
				pointers.push_back(iterator->sibling);
			}
			probability.insert(pair<int, double>(/*counter*/iterator->level, (double)iterator->counter / d->counter));
			//counter++;
			iterator = iterator->firstChildren;
		}
		iterator = pointers.back();
		//counter = iterator->level;
		pointers.pop_back();
		--counting;
	}

	for (int i = 1; i <= maxLength; ++i) {
		for (multimap<int, double>::iterator it = probability.begin(); it != probability.end(); ++it) {
			if (it->first == i) {
				entropy += it->second * log_2(it->second);
			}
		}
	}

	/*dictionary iteratorSibling = d->firstChildren;
	dictionary iteratorChildren = d->firstChildren;
	vector<dictionary> pointers;

	double entropy = 0.0;
	
	while (iteratorChildren != NULL) {
		vector<double> probability;
		while (iteratorSibling != NULL) {
			if (iteratorSibling->firstChildren != NULL) {
				pointers.push_back(iteratorSibling->firstChildren);
			}
			probability.push_back((double)iteratorSibling->counter / d->counter);
			iteratorSibling = iteratorSibling->sibling;
		}
		// FINITO IL LIVELLO, CALCOLO L'ENTROPIA DEL LIVELLO.
		for (vector<double>::iterator it = probability.begin(); it != probability.end(); ++it) {
			entropy += probability.at(it - probability.begin()) * log_2(probability.at(it - probability.begin()));
		}
		// PASSO AL LIVELLO SUCCESSIVO.
		iteratorChildren = *pointers.begin();
		iteratorSibling = *pointers.begin();
		pointers.erase(pointers.begin());		
	}
	*/
	entropy = -entropy;
	return entropy;
}

// Conta tutte le occorrenze di tutti i nodi, a tutti i livelli, con lunghezza inferiore ed uguale alla parola
// data.
// La funzione segue il percorso della parola data, non conta quindi le occorrenze di nodi che sono in rami
// dove non compaiono lettere della parola data.
vector<int> nodeAtEachLevels(dictionary d, string word) {
	int size = word.size();
	int counter = 0;

	vector<int> nodeAtLevels(size + 1);
	nodeAtLevels[counter] = 0;
	counter++;

	dictionary pointTo = d->firstChildren;

	dictionary iterator = d->firstChildren;

	while (size) {
		while (iterator != NULL) {
			if (iterator->key == (word.c_str())[counter - 1]) {
				pointTo = iterator;
			}
			nodeAtLevels[counter] += iterator->counter;
			iterator = iterator->sibling;
		}
		iterator = pointTo;
		iterator = iterator->firstChildren;
		size--;
		counter++;
	}
	return nodeAtLevels;
}

// Restituisce le occorrenze delle lettere che compongono la parola data ad ogni livello
// In posizione 0 sono presenti le occorrenze nella radice, che sono sempre 0.
// Inposizione 1 sono presenti le occorrenze al livello 1 della prima lettera che compone la parola data
// E così via.
vector<int> occurrenceAtEachLevels(dictionary d, string word) {
	int size = word.size();
	int counter = 0;

	vector<int> occurrenceAtLevels(size + 1);
	occurrenceAtLevels[counter] = 0;
	counter++;

	dictionary iterator = d->firstChildren;

	while (size) {
		while (iterator != NULL) {
			if (iterator->key == (word.c_str())[counter - 1]) {
				occurrenceAtLevels[counter] += iterator->counter;
				break;
			}
			else {
				iterator = iterator->sibling;
			}
		}
		iterator = iterator->firstChildren;
		size--;
		counter++;
	}
	return occurrenceAtLevels;
}

// Calcola l'entropia della parola data su tutti i livelli uguali ed inferiori alla lunghezza della parola
// data.
vector<double> entropyOfaWord(dictionary d, string word) {
	int size = word.size();
	int counter = 0;

	vector<double> result(size + 1);
	result[counter] = 0.0;

	vector<int> nodeAtLevels = nodeAtEachLevels(d, word);

	vector<int> occurrence = occurrenceAtEachLevels(d, word);

	dictionary iterator = d->firstChildren;

	vector<double> probability(size + 1);

	for (vector<int>::iterator it = occurrence.begin(); it != occurrence.end(); ++it) {
		probability[it - occurrence.begin()] = (double)occurrence[it - occurrence.begin()] / nodeAtLevels[it - occurrence.begin()];
	}

	probability[0] = 0.0;

	vector<double> entropy(size + 1);
	for (vector<double>::iterator it = probability.begin(); it != probability.end(); ++it) {
		entropy[it - probability.begin()] += probability[it - probability.begin()] * log_2(probability[it - probability.begin()]);
	}

	entropy[0] = 0;

	for (vector<double>::iterator it = entropy.begin(); it != entropy.end(); ++it) {
		entropy[it - entropy.begin()] = -entropy[it - entropy.begin()];
	}

	return entropy;

	/*dictionary iteratorChildren = d->firstChildren;
	dictionary iteratorSibling = d->firstChildren;
	vector<double> result;
	result.push_back(0.0);
	while (iteratorChildren != NULL) {
	int counterSibling = 0;
	map<char, int> occurrence;
	while (iteratorSibling != NULL) {
	occurrence[iteratorSibling->key] = iteratorSibling->counter;
	counterSibling += iteratorSibling->counter;
	iteratorSibling = iteratorSibling->sibling;
	}

	map<char, double> probability;
	for (map<char, int>::iterator it = occurrence.begin(); it != occurrence.end(); ++it) {
	probability[it->first] = (double)it->second / counterSibling;
	}

	double entropy = 0.0;
	for (map<char, double>::iterator it = probability.begin(); it != probability.end(); ++it) {
	entropy += it->second * log2(it->second);
	}

	entropy = -entropy;
	result.push_back(entropy);
	iteratorChildren = iteratorChildren->firstChildren;
	}*/

	return result;
}

int main() {

#if 1
	
	/*
	
	INIZIO PARTE UNO

	*/

	ifstream source("354984si.ngl");
	//ifstream source("prova.ngl");
	dictionary d = root();
	//string input[4] = { "ciao", "cima", "bob", "mam" };
	string input;
	int counterWords = 0;
	int maxLength = 0;
	//getline(source, input);
	while (getline(source, input)/*!source.eof()*/) {
		if (input.length() > maxLength) {
			maxLength = input.length();
		}
		addWord(d, input);
		//getline(source, input);
		++counterWords;
	}

	/*for (int i = 0; i < 4; ++i) {
		addWord(d, input[i]);
	}*/

	cout << "Done." << endl;

	/*

	FINE PARTE UNO

	*/

#endif

#if 1

	/*

	INIZIO PARTE DUE

	*/

	double entropy = entropyOfFirstLevel(d);
	cout << "Entropia primo livello: " << entropy << endl;

	int levelCondition = 2;

	/*
	string choosen = "nondependancies";//"ciao";
	double entropyConditionGivenCharacter = conditionalEntropyGivenCharacter(d, choosen, levelCondition);
	cout << "Entropia livello " << levelCondition << " della parola " << choosen << " e': " << entropyConditionGivenCharacter << endl;
	*/

	double entropyConditional = conditionalEntropy(d, levelCondition);
	cout << "Entropia livello " << levelCondition << " e': " << entropyConditional << endl;

	cout << "Entropia su tutti i livelli..." << endl;
	vector<double> entropyConditionalAllLevels = conditionalEntropyAllLevels(d, maxLength);
	for (vector<double>::iterator it = entropyConditionalAllLevels.begin(); it != entropyConditionalAllLevels.end(); ++it) {
		cout << "Livello " << it - entropyConditionalAllLevels.begin() << " : " << *it << endl;
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
		cout << "Inserisci una lettera..." << endl;
		cin >> character;
		map<char, double> entropyLevel = nextProbabilityNode(pointer, character);
		sort(entropyLevel.begin(), entropyLevel.end());
		for (map<char, double>::iterator it = entropyLevel.begin(); it != entropyLevel.end(); ++it) {
			cout << "Lettera " << it->first << " con probabilita': " << it->second << endl;
		}
	}

	/*

	FINE PARTE TRE

	*/

#endif

#if 0

	/*

	INIZIO PARTE QUATTRO

	*/

	double entropyDictionary = entropyOfDictionary(d, counterWords, maxLength);
	cout << "Entropia del dizionario: " << entropyDictionary << endl;

	/*

	FINE PARTE QUATTRO

	*/

#endif

	/*
	//string str[4] = { "avengeress", "sensiblest", "nondependancies", "feres" };
	string str[4] = { "ciao", "cima", "bob", "mam" };

	string choosenString = str[0];
	vector<double> entropyWord = entropyOfaWord(d, choosenString);

	cout << endl << "Si e' scelta la parola " << choosenString << endl;
	for (vector<double>::iterator it = entropyWord.begin(); it != entropyWord.end(); ++it) {
		cout << "Livello " << it - entropyWord.begin() << ": " << entropyWord[it - entropyWord.begin()] << endl;
	}
	*/

	/*
	cout << "Scegli la parola..." << endl;
	cin >> choosenString;
	cout << "Si è scelta la parola: " << choosenString << endl;

	for (vector<double>::iterator it = entropyWord.begin(); it != entropyWord.end(); ++it) {
		cout << "Livello " << it - entropyWord.begin() << ": " << entropyWord[it - entropyWord.begin()] << endl;
	}
	*/

	getchar();
	return 0;
}