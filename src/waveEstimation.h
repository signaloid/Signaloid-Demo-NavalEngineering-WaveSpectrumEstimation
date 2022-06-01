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
 *	@brief Calculate RAO from vessel characterisation measurements.
 *
 *	@param RAO           : Pointer to buffer to store RAO characteristic.
 *	@param heaveSpectrum : Pointer to buffer containing heave spectrum.
 *	@param waveSpectrum  : Pointer to buffer containing wave energy spectrum.
 *	@param N             : Number of elements in each buffer array.
 */
void
calculateRAO(
	float * const       RAO,
	const float * const heaveSpectrum,
	const float * const waveSpectrum,
	const size_t        N);

/**
 *	@brief Calculate wave energy spectrum from heave energy spectrum and RAO.
 *	@note All buffer arrays must contain the same number of elements.
 *	They should all be indexed using the same frequency scale.
 *
 *	@param waveSpectrum  : Pointer to buffer to store wave spectrum.
 *	@param heaveSpectrum : Pointer to buffer containing measured heave energy spectrum.
 *	@param RAO           : Pointer to buffer containing RAO vs frequency for the target vessel.
 *	@param N             : Number of elements in each buffer array.
 */
void
calculateWaveEnergySpectrum(
	float * const       waveSpectrum,
	const float * const heaveSpectrum,
	const float * const RAO,
	const size_t        N);
