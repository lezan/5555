#ifndef RANDOM_H
#define RANDOM_H

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <iostream>

#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
/*#define EPS MINDOUBLE*/
#define EPS DBL_MIN
#define RNMX (1.0-EPS)

using namespace std;

// a uniform random number generator between zero and 1.
class Random
{
	long idum2;
	long idum;
	long iy;
	long iv[NTAB];

	unsigned memory;
	void init(long seed);
	double ran2();
	int k;
	int rsd_max; //max val for rsd
	double rsd_pmax; // tau(i=k/R)
	double rsd_beta; // normalizing factor

public:
	Random(long seed);
	Random();
	double doublerandom();
	long longrandom(long range);
	bool boolrandom();
	double gaussian(double variance);
	void init_isd(int kk) { k = kk; }
	void init_rsd(int kk, double delta, double c); //init rsd parameters
	int isd(); // ideal Solinton distribution
	int rsd(); // robust Solinton distribution
	double get_rsd_beta() { return rsd_beta; }

	//void iid_buffer(Buffer & buf, unsigned long int len, float p0=0.5);
	//void BSC_channel(Buffer &tx, Buffer & rx, float ber=0.5);
	//void BSC_channel(Buffer &buf, float ber=0.5);
	//float * AWGN_channel(Buffer & tx,float snr=3.);
	//void AWGN_channel(Buffer & tx, float * rx, float snr=3.);
};

//double gaussian(double variance,long seed);
//void get_llr(float *soft_val, float *llr, unsigned long int numbits, float snr=3.);

#endif  // RANDOM_H
