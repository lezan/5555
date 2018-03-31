#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>

#define MAX_NBITS 16
#define N_SOURCE 3 // Numero di sorgenti: 1, 2, 3.

// 1 se voglio calcolare l'entropia per quel tipo di file, 0 se non voglio calcolarla
#define TXT 1
#define JPG 1
#define WAV 1

#define GROUPBITS 8 // Bit set size con cui voglio calcolare l'entropia

using namespace std;

double log2(double n) {
	return log(n) / log(2.0);
}

double calc(int nbits, string source) {

	int MAX_BUFFER = (int)pow(2.0, nbits); // Definisco la dimensione del buffer delle occorrenze

	ifstream fileInput(source, ios::binary); // Apro il file da leggere

	int shift = 0; // Shifter su gli 8 bit che leggo ad ogni iterazione
	vector<int> occurrenceCounter(MAX_BUFFER); //Creo il buffer delle occorrenze
	uint8_t readBits = 0; // La variabile che conterrà gli 8 bits estratti dal file
	int totalCounter = 0; // Counter per "contare" il numero totali di occorrenze
	int index = 0; // Conterrà per ogni gruppo di bits il numero letto da incrementare nel buffer delle occorrenze

	fileInput >> readBits; // Leggo da file 8 bit e gli inserisco nel contenitore

	while (!fileInput.eof()) { // Itero fino alla fine del file
		index = 0; // Re-inizializzo a zero l'index quando ho terminato di leggere un gruppo di bits (1, 2, 3...)
		for (int currentShift = 0; currentShift < nbits; ++currentShift) { // Itero un numero di volte pari al numero di bits richiesti per il bit set size
			int currentBit = ((readBits >> shift) & 1); // Estraggo il bit necessario
			index |= currentBit << currentShift; // Ricavo l'indice dell'elemento estratto da incrementare nel buffer delle occorrenze
			shift++; // Incremento lo shifter dato che sto avanzando 
			if (shift == (sizeof(readBits) * 8)) { // E' true se ho finito di leggere tutti gli 8 bits estratti
				fileInput >> readBits; // Ri-leggo altri 8 bits, dato che ho finito con i precedenti
				shift = 0; // Re-inizializzo a zero lo shift, dato che ho finito di leggere gli 8 bits
			}
		}
		occurrenceCounter[index]++; // Incremento il buffer delle occorrenze
		totalCounter++; // Incremento il counter degli elementi "contati" 
	}

	fileInput.close();

	vector<double> probability(MAX_BUFFER); // Creo il buffer delle probabilità

	for (int i = 0; i < MAX_BUFFER; ++i) { // Calcolo le probabilità di tutte le occorrenze presenti nel buffer delle occorrenze
		probability[i] = (double)occurrenceCounter[i] / totalCounter;
	}

	double entropy = 0.0; // Definisco la variabile dell'entropia

	for (int i = 0; i < MAX_BUFFER; ++i) { // Calcolo l'entropia su tutte le probabilità 
		if (probability[i] != 0) { // Escludo quelle probabilità che sono zero
			entropy += probability[i] * log2(probability[i]); // Calcolo effettivamente l'entropia totale
		}
	}

	// "Sistemo" l'entropia
	entropy /= nbits;
	entropy = -entropy;

	return entropy;
}

int main() {

	string source[3] = {"test.txt", "test.jpg", "test.wav"};
	ofstream outfile("bench.txt", ofstream::binary);
	ofstream outfileCSV("bench.csv", ofstream::binary);

	ifstream zipTxt("test.txt.zip", ios::binary | ios::ate);
	streampos sizeOfZipTxt = zipTxt.tellg();
	zipTxt.close();
	ifstream zipJpg("test.jpg.zip", ios::binary | ios::ate);
	streampos sizeOfZipJpg = zipJpg.tellg();
	zipJpg.close();
	ifstream zipWav("test.wav.zip", ios::binary | ios::ate);
	streampos sizeOfZipWav = zipWav.tellg();
	zipWav.close();
	ifstream txt("test.txt", ios::binary | ios::ate);
	streampos sizeOfTxt = txt.tellg();
	txt.close();
	ifstream jpg("test.jpg", ios::binary | ios::ate);
	streampos sizeOfJpg = jpg.tellg();
	jpg.close();
	ifstream wav("test.wav", ios::binary | ios::ate);
	streampos sizeOfWav = wav.tellg();
	wav.close();

	double CRTxt = (double)sizeOfTxt / sizeOfZipTxt;
	double CRJpg = (double)sizeOfJpg / sizeOfZipJpg;
	double CRWav = (double)sizeOfWav / sizeOfZipWav;
	
	for (int j = 0; j < N_SOURCE; ++j) {
#if 1 // Se voglio creare i files e calcolo l'entropia per tutti i gruppi di bits fino a 16 (compreso)
		if (j == 0) {
			cout << "Entropy for TXT file" << endl << endl;
			outfile << "TXT" << endl;
		}

		if (j == 1) {
			cout << endl << "Entropy for JPG file" << endl << endl;
			outfile << "JPG" << endl;
		}

		if (j == 2) {
			cout << endl << "Entropy for WAV file" << endl << endl;
			outfile << "WAV" << endl;
		}

		for (int i = 1; i <= 16; ++i) {
			double entropy = calc(i, source[j]);
			outfile << i << " bits: \t" << "Entropy: \t" <<entropy << "\t CR: \t" << 1 / entropy << "\t CR Zip: \t";
			outfileCSV << i << "," << entropy << "," << 1 / entropy << ",";
			cout << "# " << i << endl << "Entropy: \t\t" << entropy << endl;
			cout << "Compression rate: \t" << 1 / entropy << endl; 
			if (j == 0) {
				cout << "Compression rate zip: \t" << CRTxt << endl << endl;
				outfile << CRTxt << endl;
				outfileCSV << CRTxt << endl;
			}

			if (j == 1) {
				cout << "Compression rate zip: \t" << CRJpg << endl << endl;
				outfile << CRJpg << endl;
				outfileCSV << CRJpg << endl;
			}

			if (j == 2) {
				cout << "Compression rate zip: \t" << CRWav << endl << endl;
				outfile << CRWav << endl;
				outfileCSV << CRWav<< endl;
			}
		}
		outfile << " ------------------------------------------------ " << endl;
#else // Se voglio solo stampare a video i valori dell'entropia per un solo gruppo di bits
		if (TXT) {
			double entropy = calc(GROUPBITS, source[j]);
			cout << "Bits " << GROUPBITS << " TXT: \t" << entropy << endl;
			cout << "Compression rate: \t" << entropy / GROUPBITS << endl;
		}
		if (JPG) {
			double entropy = calc(GROUPBITS, source[j]);
			cout << "Bits " << GROUPBITS << " JPG \t" << entropy << endl;
			cout << "Compression rate: \t" << entropy / GROUPBITS << endl;
		}
		if (WAV) {
			double entropy = calc(GROUPBITS, source[j]);
			cout << "Bits " << GROUPBITS << " WAV: \t" << entropy << endl;
			cout << "Compression rate: \t" << entropy / GROUPBITS << endl;
		}
#endif
	}
	outfile.close();
	outfileCSV.close();

	cout << "Done!" << endl;
	getchar();
	return 0;
}
