#include "random.h"
/* Long period (? 2 \Theta 10 18 ) random number generator of
L'Ecuyer with Bays�Durham shuffle and added safeguards. Returns a
uniform random deviate between 0.0 and 1.0 (exclusive of the
endpoint values). Call with idum a negative integer to initialize;
thereafter, do not alter idum between successive deviates in a
sequence. RNMX should approximate the largest floating value that
is less than 1. -- */

//Random before;




double Random::ran2() {
	int j;
	long k;
	double temp;


	k = (idum) / IQ1;
	idum = IA1*(idum - k*IQ1) - k*IR1;  // Compute idum=(IA1*idum) % IM1 without overflows by Schrage's method.
	if (idum < 0)
		idum += IM1;
	k = idum2 / IQ2;
	idum2 = IA2*(idum2 - k*IQ2) - k*IR2;  // Compute idum2=(IA2*idum) % IM2 likewise.
	if (idum2 < 0)
		idum2 += IM2;
	j = iy / NDIV;
	iy = iv[j] - idum2;
	//  iy=iv[j]-idum2;         // Here idum is shuffled, idum and idum2 are combined to generate output.
	iv[j] = idum;
	if (iy < 1)
		iy += IMM1;
	if ((temp = AM*iy) > RNMX)
		return RNMX;        // Because users don't expect endpoint values.
	else
		return temp;
}


void Random::init(long seed) {
	idum2 = 123456789;
	idum = 0;
	iy = 0;

	if (seed != 0)
		idum = seed;
	else
		idum = 1;


	for (int j = NTAB + 7; j >= 0; j--) // Load the shuffle table (after 8 warm�ups).
	{
		long k = (idum) / IQ1;

		idum = IA1*(idum - k*IQ1) - k*IR1;
		if (idum < 0)
			idum += IM1;
		if (j < NTAB)
			iv[j] = idum;
	}
	iy = iv[0];
}

Random::Random(long seed) {
	init(seed);
}

Random::Random() {
	time_t t;

	time(&t);

	init((long)t);
}

double Random::doublerandom() {
	double t = ran2();
	return t;
}

long Random::longrandom(long range) {
	double t;

	t = doublerandom();
	return((long)(t*(double)range));
}

bool Random::boolrandom() {
	double t = doublerandom();

	if (t>0.5)
		return true;
	else
		return false;
}


double Random::gaussian(double variance)
{
	double returnvalue = 0;
	double k;
	k = sqrt(variance / 2.0);
	// add 24 uniform RV to obtain a simulation of normality
	for (int x = 0; x<24; x++)
		returnvalue += ran2();

	return k*(returnvalue - 0.5 * 24);

}


int Random::isd()
{
	int i;
	double v, range;
	v = ran2();

	range = 1.0 / (double)k;
	if (v <= range)
		return 1;
	else {
		for (i = 2; i<k; i++) {
			range = 1.0 / (double)i;
			if (v>range)
				return i;
		}
	}
	return k;
}

void Random::init_rsd(int kk, double delta, double c)
{
	double R;
	int i;
	k = kk;
	R = c*log((double)k / delta)*sqrt((double)k);
	rsd_max = floor((double)k / R + 0.5);
	rsd_max = (rsd_max > k) ? k : rsd_max;
	R = (double)k / (double)rsd_max;
	rsd_pmax = R*log(R / delta) / (double)k;

	for (i = 1, rsd_beta = 1.0; i<rsd_max; i++)
		rsd_beta += R / (double)(i*k);
	rsd_beta += rsd_pmax;
	cout << "RSD intialized: " << endl;
	cout << "dmax=" << rsd_max << " Overhead = " << rsd_beta << endl;
	return;
}

int Random::rsd()
{
	double v, range;
	int i;
	v = ran2();

	range = (rsd_pmax + 1.0 / (double)(rsd_max*(rsd_max - 1))) / rsd_beta;
	if (v <= range)
		return rsd_max;
	range += (1 / (double)k + 1 / (double)rsd_max) / rsd_beta;

	if (v <= range)
		return 1;
	for (i = 2; i<rsd_max; i++) {
		range += (1 / (double)(rsd_max*i) + 1.0 / (double)(i*(i - 1))) / rsd_beta;
		if (v <= range)
			return i;
	}

	for (i = rsd_max + 1; i<k - 1; i++) {
		range += (1.0 / (double)(i*(i - 1))) / rsd_beta;
		if (v <= range)
			return i;
	}
	return k;
}

//void Random::iid_buffer(Buffer & buf, unsigned long int len,float p0)
//{
//unsigned long int i,pos;
//pos=buf.get_position();
//for(i=0;i<len;i++)
//	buf.bufferbit((int)(ran2()>p0));
//buf.set_position(pos);
//return;
//}
//
//void Random::BSC_channel(Buffer &tx, Buffer & rx, float ber)
//{
//unsigned long int i,pos_rx,pos_tx,len;
//int bit;
//pos_tx=tx.get_position();
//pos_rx=rx.get_position();
//len=tx.remaining_bits();
//for(i=0;i<len;i++)
//{
//	bit=tx.getbit();
//	bit^=(int)(ran2()<ber);
//	rx.bufferbit(bit);
//}
//tx.set_position(pos_tx);
//rx.set_position(pos_rx); 
//return;
//}
//
//void Random::BSC_channel(Buffer &buf, float ber)
//{
//unsigned long int i,pos,len;
//int bit;
//pos=buf.get_position();
//len=buf.remaining_bits();
//for(i=0;i<len;i++)
//{
//	//bit = buf[pos+i] ^ (int)(ran2()<ber);
//	bit = buf.getbit() ^ (int)(ran2()<ber);
//	buf[pos+i]=bit;
//}
//buf.set_position(pos);
//return;
//}
//
//float * Random::AWGN_channel(Buffer &tx, float snr)
//{
//	float *rx;
//	int i,bit;
//	unsigned long int len,pos;
//	float var;
//
//	var=0.5f*(float)pow(10.f,-snr/10.f);
//	pos=tx.get_position();
//	len=tx.remaining_bits();
//	rx= new float[len];
//	
//	for(i=0;i<len;i++)
//	{
//		bit = tx.getbit();
//		rx[i] = (float)((2.*(double)bit -1.) + gaussian((double)var));
//	}
//	tx.set_position(pos);
//	return(rx);
//}
//
//
//void Random::AWGN_channel(Buffer &tx, float *rx, float snr)
//{
//	int i,bit;
//	unsigned long int len,pos;
//	float var;
//
//	var=0.5f*(float)pow(10.,-snr/10.);
//
//	pos=tx.get_position();
//	len=tx.remaining_bits();
//
//	
//	for(i=0;i<len;i++)
//	{
//		bit = tx.getbit();
//		rx[i] = (float)((2.*(double)bit -1.) + gaussian((double)var));
//	}
//	tx.set_position(pos);
//	return;
//}
//
//
//void get_llr(float *soft_val, float *llr, unsigned long int numbits,float snr)
//{
//	float var;
//	int i;
//	var=0.5f*(float)pow(10.,-snr/10.);
//	for(i=0;i<numbits;i++)
//	{
//		llr[i]= -(soft_val[i]-1.)*(soft_val[i]-1.) + (soft_val[i]+1.)*(soft_val[i]+1.);
//		llr[i]/=var;
//	}
//}

/*double gaussian(double variance, long seed) {
// static becuase we don't want to have it initialized each time we go in
Random before(seed);
double returnvalue=0;
double k;

k = sqrt(variance/2.0);

// add 24 uniform RV to obtain a simulation of normality
for (int x=0;x<24;x++)
returnvalue += before.doublerandom();

return k*(returnvalue-0.5*24);
}*/