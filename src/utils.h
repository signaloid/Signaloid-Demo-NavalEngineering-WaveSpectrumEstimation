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
 *	@brief Buffer array stored in the heap.
 *
 */
typedef struct Buffer
{
	float * heapPointer;
	size_t  size;
} Buffer;

/**
 *	@brief Subtract the mean value of a Buffer from all elements in the Buffer.
 *
 *	@param buf : Pointer to Buffer to modify.
 */
void
subtractMean(Buffer * const buf);

/**
 *	@brief Read floats from a CSV file to a heap Buffer.
 *
 *	@param filePath : Path to CSV file.
 *	@param buf      : Pointer to Buffer to store information read from file.
 *	@return int     : Return code (0 if OK, 1 if error encountered)
 */
int
readFloatsFromFileToHeapBuffer(const char * const filePath, Buffer * const buf);

/**
 *	@brief Extend the size of a heap buffer.
 *
 *	@param buf     : Pointer to buffer to modify
 *	@param newSize : New number of floats in the buffer
 *	@return int    : 0 if success, else 1
 */
int
extendHeapBuffer(Buffer * const buf, const size_t newSize);

/**
 *	@brief Deallocate the heap memory used to store the contents of a Buffer.
 *
 *	@param buf : Pointer to Buffer to free.
 */
void
freeHeapBuffer(Buffer * const buf);

/**
 *	@brief Perform double integration to convert acceleration values to position values.
 *
 *	@param timeSeriesData : Pointer to Buffer containing time series acceleration data.
 *	@param dt             : Timestep value (time between successive time series data points).
 */
void
numericalIntegration(Buffer * const timeSeriesData, const float dt);
