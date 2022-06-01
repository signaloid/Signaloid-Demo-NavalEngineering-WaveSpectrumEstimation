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

#pragma once

#include <stddef.h>

/**
 *	@brief Round up a size_t value to the next highest power of two.
 *
 *	@param arg : Value to round up.
 *	@return size_t : Next highest power of two. Note that 0 is returned if the input argument is
 *	too large for the return value to fit into a size_t variable.
 */
size_t
roundUpToNextHighestPowerOfTwo(const size_t arg);

/**
 *	@brief Calculate power spectrum of time series data.
 *	@note Time series data is zero padded so that the array size is a power of two.
 *	The power spectrum buffer must be at least the same size as the zero padded time
 *	series data array.
 *
 *	@param powerSpectrum  : Pointer to buffer to store power spectrum.
 *	@param timeSeriesData : Pointer to buffer containing time series data.
 *	@param N              : Number of elements in the time series data array.
 *	@return int : 0 if success, 1 if error encountered
 */
int
calculatePowerSpectrum(
	float * const       powerSpectrum,
	const float * const timeSeriesData,
	const size_t        N);

/**
 *	@brief Calculate periodogram from frequency spectrum.
 *
 *	@param S : Pointer to buffer to store power spectrum (periodogram).
 *	This buffer must be the same size as the frequency spectrum buffer.
 *	@param F : Pointer to buffer containing frequency spectrum.
 *	@param N : Number of elements in frequency spectrum array.
 */
void
periodogram(float * const S, const float * const F, const size_t N);

/**
 *	@brief Perform FFT on time series data.
 *	@note Time series data is zero padded so that array size is a power of two.
 *	The frequency spectrum buffer must be at least the same size as the zero padded time series
 *	data buffer.
 *
 *	@param F : Pointer to buffer to store frequency spectrum.
 *	@param x : Pointer to buffer containing time series data.
 *	@param N : Number of elements in time series data array.
 *	@return int : 0 if success, 1 if error encountered.
 */
int
fft(float * const F, const float * const x, const size_t N);
