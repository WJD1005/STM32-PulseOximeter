#ifndef __ALGORITHM_H
#define __ALGORITHM_H

typedef struct
{
	float Real;
	float Imag;
}Complex;

double Floor(double x);
double Mod(double x, double y);
double Sin(double x);
double Cos(double x);
int Sqrt(int x);
Complex EE(Complex x, Complex y);
void FFT(Complex *xin, uint16_t n);
int Find_MaxIndex(Complex *Data, uint16_t n, uint16_t Start);

#endif
