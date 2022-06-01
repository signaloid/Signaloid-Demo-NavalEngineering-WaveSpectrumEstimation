/*
 *	Authored 2022, Greg Brooks.
 *
 *	Copyright (c) 2022, Signaloid.
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#include "signalProcessing.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Complex
{
	float real;
	float imaginary;
} Complex;

static void
complexAdd(Complex * const result, const Complex * const A, const Complex * const B)
{
	result->real = A->real + B->real;
	result->imaginary = A->imaginary + B->imaginary;
}

static void
complexSubtract(Complex * const result, const Complex * const A, const Complex * const B)
{
	result->real = A->real - B->real;
	result->imaginary = A->imaginary - B->imaginary;
}

static void
complexMultiply(Complex * result, const Complex * const A, const Complex * const B)
{
	const float a = A->real;
	const float b = A->imaginary;
	const float c = B->real;
	const float d = B->imaginary;
	result->real = a * c - b * d;
	result->imaginary = a * d + b * c;
}

static void
complexExponent(Complex * const result, const float angle)
{
	result->real = cosf(angle);
	result->imaginary = sinf(angle);
}

static float
complexMagnitude(const Complex * const A)
{
	const float a = A->real;
	const float b = A->imaginary;
	return sqrtf(a * a + b * b);
}

/**
 *	@brief Perform radix-2 DIT FFT.
 *	@note Frequency spectrum Buffer must have same size as time series data buffer.
 *	@note The number of elements in each buffer must be a power of two.
 *
 *	@param F : Pointer to buffer to store frequency spectrum.
 *	@param x : Pointer to buffer containing time series data.
 *	@param N : Number of elements in time series data buffer.
 *	@param stride : Stride of input array.
 */
static void
dit2FFT(Complex * const F, const Complex * const x, const size_t N, const size_t stride)
{
	double PI = acos(-1);

	if (N == 1)
	{
		F[0].real = x[0].real;
		F[0].imaginary = x[0].imaginary;
	}
	else
	{
		dit2FFT(F, x, N / 2, 2 * stride);
		dit2FFT(&F[N / 2], &x[stride], N / 2, 2 * stride);

		for (size_t k = 0; k < N / 2; k++)
		{
			Complex p;
			Complex q;
			Complex temp;

			p.real = F[k].real;
			p.imaginary = F[k].imaginary;

			complexExponent(&temp, -2.0 * PI * k / N);
			complexMultiply(&q, &temp, &F[k + N / 2]);

			complexAdd(&F[k], &p, &q);
			complexSubtract(&F[k + N / 2], &p, &q);
		}
	}
}

size_t
roundUpToNextHighestPowerOfTwo(const size_t arg)
{
	const size_t maximumArgument = SIZE_MAX / 2 + 1;
	size_t       returnValue = 1;

	if (arg >= maximumArgument)
	{
		/*
		 *	Error: argument too large
		 */
		return 0;
	}

	while (returnValue < arg)
	{
		returnValue *= 2;
	}

	return returnValue;
}

int
calculatePowerSpectrum(
	float * const       powerSpectrum,
	const float * const timeSeriesData,
	const size_t        N)
{
	const size_t  spectrumSize = roundUpToNextHighestPowerOfTwo(N);
	float * const frequencySpectrum = (float *)calloc(spectrumSize, sizeof(float));

	if (frequencySpectrum == NULL || fft(frequencySpectrum, timeSeriesData, N))
	{
		/*
		 *	Failed to allocate heap memory
		 */
		printf("Error: The program ran out of heap memory. Try reducing the amount of "
		       "input data, or increasing the amount of available memory by selecting a "
		       "different core.\n");
		return 1;
	}

	periodogram(powerSpectrum, frequencySpectrum, spectrumSize);

	free(frequencySpectrum);

	return 0;
}

void
periodogram(float * const S, const float * const F, const size_t N)
{
	for (size_t i = 0; i < N; i++)
	{
		S[i] = F[i] * F[i];
	}
}

int
fft(float * const F, const float * const x, const size_t N)
{
	const size_t    spectrumSize = roundUpToNextHighestPowerOfTwo(N);
	Complex * const paddedX = (Complex *)calloc(spectrumSize, sizeof(Complex));
	Complex * const Fcplx = (Complex *)calloc(spectrumSize, sizeof(Complex));

	if (paddedX == NULL || Fcplx == NULL)
	{
		return 1;
	}

	for (size_t i = 0; i < N; i++)
	{
		paddedX[i].real = x[i];
	}

	dit2FFT(Fcplx, paddedX, spectrumSize, 1);

	for (size_t i = 0; i < spectrumSize; i++)
	{
		F[i] = complexMagnitude(&Fcplx[i]);
	}

	free(paddedX);
	free(Fcplx);

	return 0;
}
