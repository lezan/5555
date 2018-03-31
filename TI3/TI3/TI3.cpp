#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <fstream>
#include <ctime>

#define VERBOSE 0
#define ALTERNATIVE 0
#define PARTE1 0
#define ESP1 0
#define ESP2 0
#define ESP3 1

using namespace std;

double log_2(double n) {
	return log(n) / log(2.0);
}

vector<int> generatorSymbol(int nSymbols, vector<double> probabilities, int length) {
	vector<int> symbols;
	random_device rd;
	mt19937 gen(rd());
	std::size_t i(0);
	discrete_distribution<int> distribution(probabilities.size(),
		0.0,
		1.0,
		[&probabilities, &i](double)
	{
		auto w = probabilities[i];
		++i;
		return w;
	});
	for (int i = 0; i < length; ++i) {
		int symbol = distribution(gen);
		symbols.push_back(symbol);
	}

	return symbols;
}

vector<double> generateProbability(int nSymbols) {
	vector<double> listProbabilities;
	if (nSymbols == 2) {
		listProbabilities.push_back(0.4);
		listProbabilities.push_back(0.6);
	}
	else if (nSymbols == 3) {
		listProbabilities.push_back(0.2);
		listProbabilities.push_back(0.3);
		listProbabilities.push_back(0.5);
	}
	else if (nSymbols == 4) {
		listProbabilities.push_back(0.1);
		listProbabilities.push_back(0.2);
		listProbabilities.push_back(0.2);
		listProbabilities.push_back(0.5);
	}
	else if (nSymbols == 5) {
		listProbabilities.push_back(0.1);
		listProbabilities.push_back(0.1);
		listProbabilities.push_back(0.2);
		listProbabilities.push_back(0.2);
		listProbabilities.push_back(0.4);
	}
	else if (nSymbols == 6) {
		listProbabilities.push_back(0.1);
		listProbabilities.push_back(0.1);
		listProbabilities.push_back(0.15);
		listProbabilities.push_back(0.15);
		listProbabilities.push_back(0.2);
		listProbabilities.push_back(0.3);
	}
	else if (nSymbols == 7) {
		listProbabilities.push_back(0.05);
		listProbabilities.push_back(0.10);
		listProbabilities.push_back(0.10);
		listProbabilities.push_back(0.15);
		listProbabilities.push_back(0.15);
		listProbabilities.push_back(0.20);
		listProbabilities.push_back(0.25);
	}
	else {
		listProbabilities.push_back(0.0);
	}
	return listProbabilities;
}

int check(vector<int> symbols, int length) {
	int counter0 = 0;
	int counter1 = 0;
	double probability0 = 0.0;
	double probability1 = 0.0;
	for (auto it : symbols) {
		if (it == 0) {
			++counter0;
		}
		else {
			++counter1;
		}
	}
#if VERBOSE
	cout << "0: " << counter0 << endl << "1: " << counter1 << endl;
#endif
	if (counter0 + counter1 != length) {
		return 1;
	}
	probability0 = (double)counter0 / length;
	probability1 = (double)counter1 / length;
#if VERBOSE
	cout << "0: " << probability0 << endl << "1: " << probability1 << endl;
#endif
	if (probability0 + probability1 == 1) {
		return 2;
	}
	return 0;
}

double highRange(int symbol, vector<double> cumulativeProbabilities, int nSymbols) {
	double result = 0.0;
	for (int i = 0; i < nSymbols; ++i) {
		if (i == symbol) {
			result = cumulativeProbabilities[i + 1];
		}
	}
	return result;
}

double lowRange(int symbol, vector<double> cumulativeProbabilities, int nSymbols) {
	double result = 0.0;
	for (int i = 0; i < nSymbols; ++i) {
		if (i == symbol) {
			result = cumulativeProbabilities[i];
		}
	}
	return result;
}

double compressOnlyLow(vector<int> symbols, vector<double> cumulativeProbabilities, int nSymbols) {
	double low = 0.0;
	double high = 1.0;

	for (auto it : symbols) {
		double range = high - low;
		high = low + range * highRange(it, cumulativeProbabilities, nSymbols);
		low = low + range * lowRange(it, cumulativeProbabilities, nSymbols);
	}
	return low;
}

vector<double> compressLowHigh(vector<int> symbols, vector<double> cumulativeProbabilities, int nSymbols) {
	double low = 0.0;
	double high = 1.0;

	vector<double> result;
	for (auto it : symbols) {
		double range = high - low;
		high = low + range * highRange(it, cumulativeProbabilities, nSymbols);
		low = low + range * lowRange(it, cumulativeProbabilities, nSymbols);
	}

	result.push_back(low);
	result.push_back(high);
	return result;
}

vector<double> compressLowHighCounter(vector<int> symbols, vector<double> cumulativeProbabilities, int nSymbols) {
	double low = 0.0;
	double high = 1.0;

	int counter = 0;

	vector<double> result;
	for (auto it : symbols) {
		if (low == high) {
			break;
		}
		++counter;
		double range = high - low;
		high = low + range * highRange(it, cumulativeProbabilities, nSymbols);
		low = low + range * lowRange(it, cumulativeProbabilities, nSymbols);
	}

	result.push_back(low);
	result.push_back(high);
	result.push_back(counter);
	return result;
}

vector<double> compressLowHighCounterWithNormalization(vector<int> symbols, vector<double> cumulativeProbabilities, int nSymbols) {
	double low = 0.0;
	double high = 1.0;

	int counterUnderflow = 0;

	int counterR1 = 0;
	int counterR2 = 0;

	vector<double> result;
	for (auto it : symbols) {
		if (low == high) {
			break;
		}
		++counterUnderflow;
		double range = high - low;
		high = low + range * highRange(it, cumulativeProbabilities, nSymbols);
		low = low + range * lowRange(it, cumulativeProbabilities, nSymbols);
		if (low >= 0.0 && high < 0.5) {
			low = 2 * low;
			high = 2 * high;
			++counterR1;
		}
		if (low >= 0.5 && high < 1.0) {
			low = 2 * (low - 0.5);
			high = 2 * (high - 0.5);
			++counterR2;
		}
	}

	result.push_back(low);
	result.push_back(high);
	result.push_back(counterUnderflow);
	result.push_back(counterR1);
	result.push_back(counterR2);
	return result;
}

vector<double> compressLowHighAdaptive(vector<int> symbols, vector<double> cumulativeProbabilities, int nSymbols) {
	double low = 0.0;
	double high = 1.0;

	int counter0 = 0;
	int totalCounter = 0;
	double p = 0.0;
	vector<double> probability = cumulativeProbabilities;

	vector<double> result;
	for (auto it : symbols) {
		++totalCounter;
		if (it == 0) {
			++counter0;
		}
		double range = high - low;
		high = low + range * highRange(it, probability, nSymbols);
		low = low + range * lowRange(it, probability, nSymbols);
		p = (double)(1 + counter0) / (2 + totalCounter);
		probability[1] = p;
	}

	result.push_back(low);
	result.push_back(high);
	return result;
}

vector<double> compressLowHighAdaptiveWithNormalization(vector<int> symbols, vector<double> cumulativeProbabilities, int nSymbols) {
	double low = 0.0;
	double high = 1.0;

	int counterUnderflow = 0;

	int counterR1 = 0;
	int counterR2 = 0;

	int counter0 = 0;
	int totalCounter = 0;
	double p = 0.0;
	vector<double> probability = cumulativeProbabilities;

	vector<double> result;
	for (auto it : symbols) {
		if (low == high) {
			break;
		}
		++counterUnderflow;
		++totalCounter;
		if (it == 0) {
			++counter0;
		}
		double range = high - low;
		high = low + range * highRange(it, probability, nSymbols);
		low = low + range * lowRange(it, probability, nSymbols);
		if (low >= 0.0 && high < 0.5) {
			low = 2 * low;
			high = 2 * high;
			++counterR1;
		}
		if (low >= 0.5 && high < 1.0) {
			low = 2 * (low - 0.5);
			high = 2 * (high - 0.5);
			++counterR2;
		}
		p = (double)(1 + counter0) / (2 + totalCounter);
		probability[1] = p;
	}

	result.push_back(low);
	result.push_back(high);
	result.push_back(counterUnderflow);
	result.push_back(counterR1);
	result.push_back(counterR2);
	return result;
}

void beforeExperiment(vector<double> listProbabilities, int length) {
	int nSymbols = 2;
	cout << "Non adattivo" << endl;
	cout << "Lunghezza input: " << length << endl;

	cout.precision(15);

	vector<int> symbols = generatorSymbol(nSymbols, listProbabilities, length);

	//check(symbols, length);

	vector<double> cumulativeProbabilities;
	cumulativeProbabilities.push_back(0.0);
	cumulativeProbabilities.push_back(*listProbabilities.begin());
	cumulativeProbabilities.push_back(*listProbabilities.begin() + *(listProbabilities.end() - 1));

	vector<double> compressed = compressLowHigh(symbols, cumulativeProbabilities, nSymbols);
	cout << "Compress: " << compressed[0] << endl;

	if (compressed[1] - compressed[0] != 0) {
		double lengthCoded = log_2(1 / (compressed[1] - compressed[0])) + 1;
		cout << "Stima lunghezza: " << lengthCoded << endl;

		double rate = lengthCoded / length;
		cout << "Rate di codifica: " << rate << endl;
	}
	else {
		cout << "Underflow." << endl;
	}
}

void checkUnderflow(int nSymbols, int length, ofstream& underflowFile, int nIteration) {
	// Può variare il NUMERI DI SIMBOLI e le PROBABILITA'
	// Quello che rimane invariato è la LUNGHEZZA del messaggio da codificare, che deve essere sufficientemente grande da generare underflow.

	underflowFile << "# symbols: " << nSymbols << endl << endl;

	vector<double> listProbabilities = generateProbability(nSymbols);
	vector<int> symbols = generatorSymbol(nSymbols, listProbabilities, length);

	vector<double> cumulativeProbabilities(nSymbols + 1);
	cumulativeProbabilities[0] = 0.0;
	for (vector<double>::iterator it = listProbabilities.begin(); it != listProbabilities.end(); ++it) {
		cumulativeProbabilities[it - listProbabilities.begin() + 1] = cumulativeProbabilities[it - listProbabilities.begin()] + *it;
	}

	for (int i = 0; i < nSymbols; ++i) {
		underflowFile << "P" << i << ": " << cumulativeProbabilities[i + 1] - cumulativeProbabilities[i] << ", ";
#if VERBOSE
		cout << "P" << i << ": " << cumulativeProbabilities[i + 1] - cumulativeProbabilities[i] << ", ";
#endif
	}
	underflowFile << ", length: " << length << " => ";
#if VERBOSE
	cout << ", length: " << length << " => ";
#endif

	//int counterUnderflow = 0;
	double counter = 0;

	for (int i = 0; i < nIteration; ++i) {
		vector<double> compressed = compressLowHighCounter(symbols, cumulativeProbabilities, nSymbols);
		if (compressed[1] - compressed[0] == 0) {
			counter += compressed[2];
			//++counterUnderflow;
#if VERBOSE
			cout << "Counter: " << compressed[2] << endl << endl;
#endif			
		}
		else {
#if VERBOSE
			cout << "No underflow" << endl;
#endif
		}
	}

	counter /= nIteration;// -counterUnderflow;

	underflowFile << "Counter: " << counter << endl << endl;
}

void checkUnderflowWithNormalization(int nSymbols, int length, ofstream& underflowFile, int nIteration) {
	// Può variare il NUMERI DI SIMBOLI e le PROBABILITA'
	// Quello che rimane invariato è la LUNGHEZZA del messaggio da codificare, che deve essere sufficientemente grande da generare underflow.

	underflowFile << "# symbols: " << nSymbols << endl << endl;

	vector<double> listProbabilities = generateProbability(nSymbols);
	vector<int> symbols = generatorSymbol(nSymbols, listProbabilities, length);

	vector<double> cumulativeProbabilities(nSymbols + 1);
	cumulativeProbabilities[0] = 0.0;
	for (vector<double>::iterator it = listProbabilities.begin(); it != listProbabilities.end(); ++it) {
		cumulativeProbabilities[it - listProbabilities.begin() + 1] = cumulativeProbabilities[it - listProbabilities.begin()] + *it;
	}

	for (int i = 0; i < nSymbols; ++i) {
		underflowFile << "P" << i << ": " << cumulativeProbabilities[i + 1] - cumulativeProbabilities[i] << ", ";
#if VERBOSE
		cout << "P" << i << ": " << cumulativeProbabilities[i + 1] - cumulativeProbabilities[i] << ", ";
#endif
	}
	underflowFile << ", length: " << length << " => ";
#if VERBOSE
	cout << ", length: " << length << " => ";
#endif

	double counter = 0;
	//int counterUnderflow = 0;

	for (int i = 0; i < nIteration; ++i) {
		vector<double> compressed = compressLowHighCounterWithNormalization(symbols, cumulativeProbabilities, nSymbols);

		if (compressed[1] - compressed[0] == 0) {
			counter += compressed[2];
			//++counterUnderflow;
#if VERBOSE
			cout << "Counter: " << compressed[2] << endl;
#endif
		}
		else {
#if VERBOSE
			cout << "No underflow" << endl;
#endif
		}
	}

	counter /= nIteration;// -counterUnderflow;

	underflowFile << "Counter: " << counter << endl << endl;
}

void checkEfficiency(int nSymbols, int maxLength, ofstream& lengthRateFile, ofstream& lengthFileCSV, ofstream& rateFileCSV, int nIteration) {

	lengthRateFile << "# symbols: " << nSymbols << endl << endl;
	lengthFileCSV << "# symbols: " << nSymbols << endl << endl;
	rateFileCSV << "# symbols: " << nSymbols << endl << endl;

	vector<double> listProbabilities = generateProbability(nSymbols);

	for (int y = 1; y <= maxLength; ++y) {
		cout.precision(15);

		vector<int> symbols = generatorSymbol(nSymbols, listProbabilities, y);

		vector<double> cumulativeProbabilities(nSymbols + 1);
		cumulativeProbabilities[0] = 0.0;
		for (vector<double>::iterator it = listProbabilities.begin(); it != listProbabilities.end(); ++it) {
			cumulativeProbabilities[it - listProbabilities.begin() + 1] = cumulativeProbabilities[it - listProbabilities.begin()] + *it;
		}

		double entropy = 0.0;
		for (vector<double>::iterator it = listProbabilities.begin(); it != listProbabilities.end(); ++it) {
			entropy += *it * log_2(*it);
		}
		entropy = -entropy;

		lengthRateFile << "Lunghezza: " << y << endl;
#if VERBOSE
		cout << "Lunghezza: " << y << endl;
#endif

		double lengthCoded = 0.0;
		double efficiency = 0.0;
		int counterUnderflow = 0;
		for (int i = 0; i < nIteration; ++i) {

			vector<double> compressed = compressLowHigh(symbols, cumulativeProbabilities, nSymbols);

			if (compressed[1] - compressed[0] != 0) {
				double temp = (log_2(1 / (compressed[1] - compressed[0])) + 1);
				lengthCoded += temp;
#if VERBOSE
				cout << "Stima lunghezza: " << lengthCoded << endl;
#endif
				efficiency += (temp / y);

#if VERBOSE
				cout << "Rate di codifica: " << efficiency << endl;
				cout << "Rate di codifica: " << efficiency << " <===> "
					<< "Entropia: " << entropy << endl;
				cout << lengthCoded << "," << y << endl;
#endif
			}
			else {
				++counterUnderflow;
#if VERBOSE
				cout << "Underflow." << endl;
#endif
			}
#if VERBOSE
			cout << " ------------------ " << endl << endl;
#endif
		}

		lengthCoded /= (nIteration - counterUnderflow);
		efficiency /= (nIteration - counterUnderflow);

		lengthRateFile << "Stima lunghezza: " << lengthCoded << endl;
		lengthRateFile << "Rate di codifica: " << efficiency << " <===> "
			<< "Entropia: " << entropy << endl << endl;
		lengthFileCSV << lengthCoded << "," << y << endl;
		rateFileCSV << efficiency << "," << entropy << endl;
	}
}

void checkEfficiencyWithNormalization(int nSymbols, int maxLength, ofstream& lengthRateFileWithNormalization, ofstream& lengthFileWithNormalizationCSV, ofstream& rateFileWithNormalizationCSV, int nIteration) {

	lengthRateFileWithNormalization << "# symbols: " << nSymbols << endl << endl;
	lengthFileWithNormalizationCSV << "# symbols: " << nSymbols << endl << endl;
	rateFileWithNormalizationCSV << "# symbols: " << nSymbols << endl << endl;

	vector<double> listProbabilities = generateProbability(nSymbols);

	for (int y = 1; y <= maxLength; ++y) {
		cout.precision(15);

		vector<int> symbols = generatorSymbol(nSymbols, listProbabilities, y);

		vector<double> cumulativeProbabilities(nSymbols + 1);
		cumulativeProbabilities[0] = 0.0;
		for (vector<double>::iterator it = listProbabilities.begin(); it != listProbabilities.end(); ++it) {
			cumulativeProbabilities[it - listProbabilities.begin() + 1] = cumulativeProbabilities[it - listProbabilities.begin()] + *it;
		}

		double entropy = 0.0;
		for (vector<double>::iterator it = listProbabilities.begin(); it != listProbabilities.end(); ++it) {
			entropy += *it * log_2(*it);
		}
		entropy = -entropy;

		lengthRateFileWithNormalization << "Lunghezza: " << y << endl;
#if VERBOSE
		cout << "Lunghezza: " << y << endl;
#endif

		double lengthCoded = 0.0;
		double efficiency = 0.0;
		int counterUnderflow = 0;

		for (int i = 0; i < nIteration; ++i) {
			vector<double> compressed = compressLowHighCounterWithNormalization(symbols, cumulativeProbabilities, nSymbols);
			if (compressed[1] - compressed[0] != 0) {
				double temp = (compressed[3] + compressed[4] + log_2(1 / (compressed[1] - compressed[0])) + 1);
				lengthCoded += temp;
#if VERBOSE
				cout << "Stima lunghezza: " << lengthCoded << endl;
#endif

				efficiency += (temp / y);
#if VERBOSE
				cout << "Rate di codifica: " << efficiency << " <===> "
					<< "Entropia: " << entropy << endl;
				cout << lengthCoded << "," << y << endl;
#endif
			}
			else {
				++counterUnderflow;
#if VERBOSE
				cout << "Underflow." << endl;
#endif
			}
#if VERBOSE
			cout << " ------------------ " << endl << endl;
#endif
		}

		lengthCoded /= (nIteration - counterUnderflow);
		efficiency /= (nIteration - counterUnderflow);

		lengthRateFileWithNormalization << "Stima lunghezza: " << lengthCoded << endl;
		lengthRateFileWithNormalization << "Rate di codifica: " << efficiency << " <===> "
			<< "Entropia: " << entropy << endl << endl;
		lengthFileWithNormalizationCSV << lengthCoded << "," << y << endl;
		rateFileWithNormalizationCSV << efficiency << "," << entropy << endl;
	}
}

void adaptive(vector<double> listProbabilities, int maxLength, bool normalization, ofstream& lengthRateFile, ofstream& lengthRateAdaptiveFileCSV, ofstream& lengthRateNonAdaptiveFileCSV) {
	int nSymbols = 2;

	lengthRateFile << "P0: " << listProbabilities[0] << ", P1: " << listProbabilities[1] << endl << endl;
	lengthRateAdaptiveFileCSV << "P0: " << listProbabilities[0] << ", P1: " << listProbabilities[1] << endl << endl;
	lengthRateNonAdaptiveFileCSV << "P0: " << listProbabilities[0] << ", P1: " << listProbabilities[1] << endl << endl;

	vector<double> cumulativeProbabilities;
	cumulativeProbabilities.push_back(0.0);
	cumulativeProbabilities.push_back(*listProbabilities.begin());
	cumulativeProbabilities.push_back(*listProbabilities.begin() + *(listProbabilities.end() - 1));

	vector<double> cumulativeProbabilitiesAdaptive;
	cumulativeProbabilitiesAdaptive.push_back(0.0);
	cumulativeProbabilitiesAdaptive.push_back(0.5);
	cumulativeProbabilitiesAdaptive.push_back(1.0);

	for (int y = 1; y <= maxLength; ++y) {

		cout << endl;
		cout << "Lunghezza input: " << y << endl << endl;
		lengthRateFile << endl;
		lengthRateFile << "Lunghezza input: " << y << endl << endl;

		lengthRateAdaptiveFileCSV << y << ",";
		lengthRateNonAdaptiveFileCSV << y << ",";

		cout.precision(15);

		vector<int> symbols = generatorSymbol(nSymbols, listProbabilities, y);

		//check(symbols, length);

		cout << "Adattivo";
		lengthRateFile << "Adattivo";

		vector<double> compressedAdaptive;
		if (normalization == false) {
			cout << ", non normalizzato" << endl;
			lengthRateFile << ", non normalizzato" << endl;
			compressedAdaptive = compressLowHighAdaptive(symbols, cumulativeProbabilitiesAdaptive, nSymbols);
		}
		else {
			cout << ", normalizzato" << endl;
			lengthRateFile << ", normalizzato" << endl;
			compressedAdaptive = compressLowHighAdaptiveWithNormalization(symbols, cumulativeProbabilitiesAdaptive, nSymbols);
		}
		cout << "Compress: " << compressedAdaptive[0] << endl;
		lengthRateFile << "Compress: " << compressedAdaptive[0] << endl;

		if (compressedAdaptive[1] - compressedAdaptive[0] != 0) {
			double lengthCoded = 0.0;
			if (normalization == false) {
				lengthCoded = log_2(1 / (compressedAdaptive[1] - compressedAdaptive[0])) + 1;
			}
			else {
				lengthCoded = compressedAdaptive[3] + compressedAdaptive[4] + log_2(1 / (compressedAdaptive[1] - compressedAdaptive[0])) + 1;
			}
			cout << "Stima lunghezza: " << lengthCoded << endl;
			lengthRateFile << "Stima lunghezza: " << lengthCoded << endl;
			lengthRateAdaptiveFileCSV << lengthCoded << ",";

			double rate = lengthCoded / y;
			cout << "Rate di codifica: " << rate << endl;
			lengthRateFile << "Rate di codifica: " << rate << endl;
			lengthRateAdaptiveFileCSV << rate << endl;
		}
		else {
			cout << "Underflow." << endl;
			lengthRateFile << "Underflow." << endl;
			lengthRateAdaptiveFileCSV << 0 << endl;
		}

		cout << endl;

		cout << "Non adattivo" << endl;
		lengthRateFile << "Non adattivo" << endl;

		double entropy = 0.0;
		for (vector<double>::iterator it = listProbabilities.begin(); it != listProbabilities.end(); ++it) {
			entropy += *it * log_2(*it);
		}
		entropy = -entropy;

		vector<double> compressedNonAdaptive = compressLowHigh(symbols, cumulativeProbabilities, nSymbols);
		cout << "Compress: " << compressedNonAdaptive[0] << endl;
		lengthRateFile << "Compress: " << compressedNonAdaptive[0] << endl;

		if (compressedNonAdaptive[1] - compressedNonAdaptive[0] != 0) {
			double lengthCoded = log_2(1 / (compressedNonAdaptive[1] - compressedNonAdaptive[0])) + 1;
			cout << "Stima lunghezza: " << lengthCoded << endl;
			lengthRateFile << "Stima lunghezza: " << lengthCoded << endl;
			lengthRateNonAdaptiveFileCSV << lengthCoded << ",";

			double rate = lengthCoded / y;
			cout << "Rate di codifica: " << rate << " <===> " << "Entropia: " << entropy << endl;
			lengthRateFile << "Rate di codifica: " << rate << " <===> " << "Entropia: " << entropy << endl;
			lengthRateNonAdaptiveFileCSV << rate << endl;
		}
		else {
			cout << "Underflow." << endl;
			lengthRateFile << "Underflow." << endl;
			lengthRateNonAdaptiveFileCSV << 0 << endl;
		}
	}
	cout << "-----------------------" << endl;
	cout << endl;
	lengthRateFile << "-----------------------" << endl;
	lengthRateFile << endl;
}

int main() {

	/*

	INIZIO PARTE 1

	*/

#if PARTE1
	int lengthParte1 = 50;
	vector<double> listProbabilitiesParte1;
	listProbabilitiesParte1.push_back(0.3);
	listProbabilitiesParte1.push_back(0.7);
	beforeExperiment(listProbabilitiesParte1, lengthParte1);
#endif

	/*

	FINE PARTE 1

	*/

	/*

	INIZIO ESPERIMENTO 1

	*/

#if ESP1
	int maxLengthEsp1 = 100;
	int lengthEsp1 = 50;
	int nIterationEsp1 = 1000;

	ofstream underflowFile("underflow.txt", ofstream::binary);
	ofstream lengthRateFile("lengthRateFile.txt", ofstream::binary);
	ofstream lengthFileCSV("lengthFile.csv", ofstream::binary);
	ofstream rateFileCSV("rateFile.csv", ofstream::binary);

	for (int nSymbolsEsp1 = 2; nSymbolsEsp1 < 8; ++nSymbolsEsp1) {
		checkUnderflow(nSymbolsEsp1, maxLengthEsp1, underflowFile, nIterationEsp1);
		checkEfficiency(nSymbolsEsp1, lengthEsp1, lengthRateFile, lengthFileCSV, rateFileCSV, nIterationEsp1);
	}

	underflowFile.close();
	lengthRateFile.close();
	lengthFileCSV.close();
	rateFileCSV.close();

	/*
	int nSymbolsEsp1 = 2;
	checkUnderflow(nSymbolsEsp1, maxLengthEsp1);
	checkEfficiency(nSymbolsEsp1, lengthEsp1);
	*/

#endif

	/*

	FINE ESPERIMENTO 1

	*/

	/*

	INIZIO ESPERIMENTO 2

	*/

#if ESP2
	int maxLengthEsp2 = 1000;
	int lengthEsp2 = 50;
	int nIterationEsp2 = 1000;

	ofstream underflowFileWithNormalization("underflowWithNormalization.txt", ofstream::binary);
	ofstream lengthRateFileWithNormalization("lengthRateFileWithNormalization.txt", ofstream::binary);
	ofstream lengthFileWithNormalizationCSV("lengthFileWithNormalization.csv", ofstream::binary);
	ofstream rateFileWithNormalizationCSV("rateFileWithNormalization.csv", ofstream::binary);

	for (int nSymbolsEsp2 = 2; nSymbolsEsp2 < 8; ++nSymbolsEsp2) {
		checkUnderflowWithNormalization(nSymbolsEsp2, maxLengthEsp2, underflowFileWithNormalization, nIterationEsp2);
		checkEfficiencyWithNormalization(nSymbolsEsp2, lengthEsp2, lengthRateFileWithNormalization, lengthFileWithNormalizationCSV, rateFileWithNormalizationCSV, nIterationEsp2);
	}

	underflowFileWithNormalization.close();
	lengthRateFileWithNormalization.close();
	lengthFileWithNormalizationCSV.close();
	rateFileWithNormalizationCSV.close();
#endif

	/*

	FINE ESPERIMENTO 2

	*/

	/*

	INIZIO ESPERIMENTO 3

	*/


#if ESP3
	int maxLengthEsp3 = 50;
	
	ofstream lengthRateFileEsp3("lengthRateFileEsp3.txt", ofstream::binary);
	ofstream lengthRateAdaptiveFileEsp3CSV("lengthRateAdaptiveFileEsp3.csv", ofstream::binary);
	ofstream lengthRateNonAdaptiveFileEsp3CSV("lengthRateNonAdaptiveFileEsp3.csv", ofstream::binary);
	srand(time(NULL));

	for (int i = 0; i < 2; ++i) {
		vector<double> listProbabilitiesEsp3;
		listProbabilitiesEsp3.push_back(((double)rand() / (RAND_MAX)));
		listProbabilitiesEsp3.push_back(1.0 - listProbabilitiesEsp3[0]);
		adaptive(listProbabilitiesEsp3, maxLengthEsp3, true, lengthRateFileEsp3, lengthRateAdaptiveFileEsp3CSV, lengthRateNonAdaptiveFileEsp3CSV);
	}

	lengthRateFileEsp3.close();
	lengthRateAdaptiveFileEsp3CSV.close();
	lengthRateNonAdaptiveFileEsp3CSV.close();
#endif

	/*

	FINE ESPERIMENTO 3

	*/

	cout << endl << "done.";
	getchar();
	return 0;
}