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

#include "utils.h"
#include "integrate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 *	@brief Determine the size of array required to store values from a CSV file.
 *
 *	@param stream  : File stream to read from.
 *	@return size_t : Size of array required to store values read from the CSV file.
 */
static size_t
determineBufferSize(FILE * stream)
{
	float  buf;
	int    fscanfResult;
	size_t count = 0;

	do
	{
		fscanfResult = fscanf(stream, "%f,", &buf);
		count++;
		if (count == 0)
		{
			/*
			 *	Overflow
			 */
			break;
		}
	} while (fscanfResult != EOF && fscanfResult != 0);

	rewind(stream);

	return --count;
}

/**
 *	@brief Read the float values from the given CSV file and store them in the specified heap
 *	buffer.
 *
 *	@param stream : File stream to read from.
 *	@param buf    : Pointer to buffer to store values read from CSV file.
 *	@return int   : Return code (0 if successful, else 1)
 */
static int
getFloatsFromCSV(FILE * stream, Buffer * const buf)
{
	for (size_t i = 0; i < buf->size; i++)
	{
		const int returnValue = fscanf(stream, "%f,", &(buf->heapPointer[i]));
		if (returnValue == EOF || returnValue == 0)
		{
			return 1;
		}
	}

	return 0;
}

void
subtractMean(Buffer * const buf)
{
	float total = 0;
	float mean;

	for (size_t i = 0; i < buf->size; i++)
	{
		total += buf->heapPointer[i];
	}

	mean = total / buf->size;

	for (size_t i = 0; i < buf->size; i++)
	{
		buf->heapPointer[i] -= mean;
	}
}

int
readFloatsFromFileToHeapBuffer(const char * const filePath, Buffer * const buf)
{
	int    returnCode;
	FILE * stream = fopen(filePath, "r");

	if (buf == NULL)
	{
		fclose(stream);
		printf("Error: null pointer passed to function readFloatsToHeapBuffer()\n");
		return 1;
	}

	if (stream == NULL)
	{
		printf("Error: could not open file at path '%s'\n", filePath);
		return 1;
	}

	buf->size = determineBufferSize(stream);

	if (buf->size == 0)
	{
		fclose(stream);
		printf("Error: no data found in the specified file ('%s')\n", filePath);
		return 1;
	}

	buf->heapPointer = (float *)calloc(buf->size, sizeof(float));

	if (buf->heapPointer == NULL)
	{
		fclose(stream);
		printf("Error: The program ran out of heap memory. Try reducing the amount of "
		       "input data, or increasing the amount of available memory by selecting a "
		       "different core.\n");
		return 1;
	}

	returnCode = getFloatsFromCSV(stream, buf);

	if (returnCode != 0)
	{
		fclose(stream);
		printf("Error: failed to read data from file at path '%s'\n", filePath);
		return 1;
	}

	fclose(stream);

	return 0;
}

int
extendHeapBuffer(Buffer * const buf, const size_t newSize)
{
	if (newSize <= buf->size)
	{
		return 0;
	}

	buf->heapPointer = reallocarray(buf->heapPointer, newSize, sizeof(float));

	if (buf->heapPointer == NULL)
	{
		buf->size = 0;
		printf("Error: The program ran out of heap memory. Try reducing the amount "
		       "of input data, or increasing the amount of available memory by selecting "
		       "a different core.\n");
		return 1;
	}

	buf->size = newSize;

	return 0;
}

void
freeHeapBuffer(Buffer * const buf)
{
	free(buf->heapPointer);
}

void
numericalIntegration(Buffer * const timeSeriesData, const float dt)
{
	State newState;
	State oldState = {
		.position = 0,
		.speed = 0,
	};
	float newAcceleration = timeSeriesData->heapPointer[0];
	float oldAcceleration = 0;

	for (size_t i = 0; i < timeSeriesData->size; i++)
	{
		newAcceleration = timeSeriesData->heapPointer[i];

		integrate(&newState, &oldState, newAcceleration, oldAcceleration, dt);

		oldAcceleration = timeSeriesData->heapPointer[i];
		timeSeriesData->heapPointer[i] = newState.position;

		oldState.position = newState.position;
		oldState.speed = newState.speed;
	}

	subtractMean(timeSeriesData);
}
