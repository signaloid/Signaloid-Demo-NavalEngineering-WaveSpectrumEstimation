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

#include "waveEstimation.h"
#include <math.h>

static void
elementWiseDivide(
	float * const       result,
	const float * const numerator,
	const float * const denominator,
	const size_t        N)
{
	for (size_t i = 0; i < N; i++)
	{
		if (denominator[i] == 0)
		{
			result[i] = INFINITY;
		}
		else
		{
			result[i] = numerator[i] / denominator[i];
		}
	}
}

void
calculateRAO(
	float * const       RAO,
	const float * const heaveSpectrum,
	const float * const waveSpectrum,
	const size_t        N)
{
	elementWiseDivide(RAO, heaveSpectrum, waveSpectrum, N);
}

void
calculateWaveEnergySpectrum(
	float * const       waveSpectrum,
	const float * const heaveSpectrum,
	const float * const RAO,
	const size_t        N)
{
	elementWiseDivide(waveSpectrum, heaveSpectrum, RAO, N);
}
