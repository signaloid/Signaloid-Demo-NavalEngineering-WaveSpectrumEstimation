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
#include "uncertain.h"
#include "utils.h"
#include "waveEstimation.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
	kMaximumPrintLinesInOutput = 9,
} Constants;

typedef struct CommandLineArguments
{
	char * heaveDisplacementFilePath;
	float  heaveMeasurementUncertainty;
	char * waveElevationFilePath;
	float  waveElevationUncertainty;
	char * heaveAccelerationFilePath;
	float  accelerometerResolution;
	float  timestep;
} CommandLineArguments;

extern char * optarg;
extern int    opterr, optopt;

/**
 *	@brief Print out command line usage.
 *
 */
static void
printUsage(void)
{
	printf("\nWave spectrum estimation example.\n");
	printf("\n");
	printf("Usage is:\n");
	printf("wave-spectrum-estimation\n"
	       "	[-d (path to heave displacement test measurements)]\n"
	       "	[-D (heave measurement uncertainty)]\n"
	       "	[-e (path to wave elevation test measurements)]\n"
	       "	[-E (wave elevation measurement uncertainty)]\n"
	       "	[-a (path to heave acceleration measurements taken at sea)]\n"
	       "	[-A (accelerometer resolution)]\n"
	       "	[-t (time between successive measurements)]\n"
	       "	[-h (display this help message)]\n");
	printf("\n");
}

/**
 *	@brief Insert measurement uncertainty into a buffer of values.
 *	@note Uncertainty is modelled with a uniform distribution here.
 *
 *	@param buffer      : Pointer to buffer
 *	@param uncertainty : measurement uncertainty (the support of the uniform distribution)
 */
static void
applyUncertainty(Buffer * const buffer, const float uncertainty)
{
	for (size_t i = 0; i < buffer->size; i++)
	{
		const float value = buffer->heapPointer[i];
		const float lowerBound = value - uncertainty / 2.0;
		const float upperBound = value + uncertainty / 2.0;

		buffer->heapPointer[i] = libUncertainFloatUniformDist(lowerBound, upperBound);
	}
}

/**
 *	@brief Characterise RAO from heave displacement and wave elevation measurements.
 *
 *	@param RAOBuffer                           : Pointer to buffer to store RAO characterisation
 *	@param heaveDisplacementFilePath           : Path to file containing heave displacement
 *	measurements
 *	@param waveElevationFilePath               : Path to file containing wave elevation
 *	measurements
 *	@param heaveMeasurementUncertainty         : Uncertainty in heave displacement measurements
 *	@param waveElevationMeasurementUncertainty : Uncertainty in wave elevation measurements
 *	@param measurementPeriod                   : Time period between successive measurements
 *	@return int : 0 if calculation is performed successfully, else 1
 */
static int
characteriseRAO(
	Buffer * const     RAOBuffer,
	const char * const heaveDisplacementFilePath,
	const char * const waveElevationFilePath,
	const float        heaveMeasurementUncertainty,
	const float        waveElevationMeasurementUncertainty,
	const float        measurementPeriod)
{
	Buffer heaveDisplacementBuffer = {
		.heapPointer = NULL,
		.size = 0,
	};
	Buffer waveElevationBuffer = {
		.heapPointer = NULL,
		.size = 0,
	};
	Buffer heaveSpectrumBuffer = {
		.heapPointer = NULL,
		.size = 0,
	};
	Buffer waveSpectrumBuffer = {
		.heapPointer = NULL,
		.size = 0,
	};
	int    returnValue = 0;
	size_t spectrumBufferSize;

	if (readFloatsFromFileToHeapBuffer(heaveDisplacementFilePath, &heaveDisplacementBuffer))
	{
		printf("Error: could not read heave displacement data from file: %s\n",
		       heaveDisplacementFilePath);
		returnValue = 1;
		goto RETURN;
	}

	if (readFloatsFromFileToHeapBuffer(waveElevationFilePath, &waveElevationBuffer))
	{
		printf("Error: could not read wave elevation data from file: %s\n",
		       waveElevationFilePath);
		returnValue = 1;
		goto RETURN;
	}

	if (heaveDisplacementBuffer.size != waveElevationBuffer.size)
	{
		printf("Error: the number of data points in the supplied heave motion and "
		       "wave elevation measurements do not match.\n"
		       "%zu measurement values in the heave motion data.\n"
		       "%zu measurement values in the wave elevation data.\n",
		       heaveDisplacementBuffer.size,
		       waveElevationBuffer.size);
		returnValue = 1;
		goto RETURN;
	}

	/*
	 *	Expand frequency domain buffers to an appropriate size.
	 */
	spectrumBufferSize = roundUpToNextHighestPowerOfTwo(heaveDisplacementBuffer.size);
	if (extendHeapBuffer(&heaveSpectrumBuffer, spectrumBufferSize) ||
	    extendHeapBuffer(&waveSpectrumBuffer, spectrumBufferSize) ||
	    extendHeapBuffer(RAOBuffer, spectrumBufferSize))
	{
		returnValue = 1;
		goto RETURN;
	}

	applyUncertainty(&heaveDisplacementBuffer, heaveMeasurementUncertainty);
	applyUncertainty(&waveElevationBuffer, waveElevationMeasurementUncertainty);

	if (calculatePowerSpectrum(
		    heaveSpectrumBuffer.heapPointer,
		    heaveDisplacementBuffer.heapPointer,
		    heaveDisplacementBuffer.size))
	{
		printf("Error: could not calculate power spectrum for heave displacement data.\n");
		returnValue = 1;
		goto RETURN;
	}

	if (calculatePowerSpectrum(
		    waveSpectrumBuffer.heapPointer,
		    waveElevationBuffer.heapPointer,
		    waveElevationBuffer.size))
	{
		printf("Error: could not calculate power spectrum for wave elevation data.\n");
		returnValue = 1;
		goto RETURN;
	}

	calculateRAO(
		RAOBuffer->heapPointer,
		heaveSpectrumBuffer.heapPointer,
		waveSpectrumBuffer.heapPointer,
		RAOBuffer->size);

RETURN:
	freeHeapBuffer(&heaveDisplacementBuffer);
	freeHeapBuffer(&waveElevationBuffer);
	freeHeapBuffer(&heaveSpectrumBuffer);
	freeHeapBuffer(&waveSpectrumBuffer);
	return returnValue;
}

/**
 *	@brief Estimate wave spectrum from accelerometer measurements and RAO.
 *
 *	@param waveSpectrumEstimateBuffer : Buffer to store wave spectrum estimate
 *	@param RAOBuffer                  : Buffer containing RAO for the vessel
 *	@param heaveAccelerationFilePath  : Path to file containing heave acceleration measurements
 *	@param accelerometerResolution    : Measurement resolution for accelerometer data
 *	@param accelerometerTimestep      : Timestep between successive accelerometer measurements
 *	@return int : 0 if calculation is performed successfully, else 1
 */
static int
estimateWaveSpectrum(
	Buffer * const       waveSpectrumEstimateBuffer,
	const Buffer * const RAOBuffer,
	const char * const   heaveAccelerationFilePath,
	float                accelerometerResolution,
	float                accelerometerTimestep)
{
	Buffer oceanHeaveBuffer = {
		.heapPointer = NULL,
		.size = 0,
	};
	Buffer heaveSpectrumBuffer = {
		.heapPointer = NULL,
		.size = 0,
	};
	int returnValue = 0;

	if (readFloatsFromFileToHeapBuffer(heaveAccelerationFilePath, &oceanHeaveBuffer))
	{
		printf("Error: could not read heave acceleration data from file: %s\n",
		       heaveAccelerationFilePath);
		returnValue = 1;
		goto RETURN;
	}

	if (oceanHeaveBuffer.size > SIZE_MAX / 2)
	{
		printf("Error: too many values in the heave acceleration input file.\n"
		       "Found %zu out of a maximum of %zu\n",
		       oceanHeaveBuffer.size,
		       SIZE_MAX / 2);
		returnValue = 1;
		goto RETURN;
	}

	/*
	 *	Insert measurement uncertainty information.
	 */
	applyUncertainty(&oceanHeaveBuffer, accelerometerResolution);

	/*
	 *	Integrate acceleration to position.
	 */
	numericalIntegration(&oceanHeaveBuffer, accelerometerTimestep);

	if (oceanHeaveBuffer.size < RAOBuffer->size)
	{
		/*
		 *	Zero pad heave buffer
		 */
		if (extendHeapBuffer(&oceanHeaveBuffer, RAOBuffer->size))
		{
			returnValue = 1;
			goto RETURN;
		}
	}

	/*
	 *	Extend other buffers to an appropriate size for element-wise arithmetic.
	 */
	if (extendHeapBuffer(&heaveSpectrumBuffer, RAOBuffer->size) ||
	    extendHeapBuffer(waveSpectrumEstimateBuffer, RAOBuffer->size))
	{
		returnValue = 1;
		goto RETURN;
	}

	/*
	 *	Calculate heave power spectrum from integrated accelerometer data.
	 */
	if (calculatePowerSpectrum(
		    heaveSpectrumBuffer.heapPointer,
		    oceanHeaveBuffer.heapPointer,
		    heaveSpectrumBuffer.size))
	{
		printf("Error: failed to calculate heave motion power spectrum\n");
		returnValue = 1;
		goto RETURN;
	}

	calculateWaveEnergySpectrum(
		waveSpectrumEstimateBuffer->heapPointer,
		heaveSpectrumBuffer.heapPointer,
		RAOBuffer->heapPointer,
		RAOBuffer->size);

RETURN:
	freeHeapBuffer(&oceanHeaveBuffer);
	freeHeapBuffer(&heaveSpectrumBuffer);
	return returnValue;
}

/**
 *	@brief Get command line arguments.
 *
 *	@param argc      : argument count from main()
 *	@param argv      : argument vector from main()
 *	@param arguments : Pointer to struct to store arguments
 *	@return int : 0 if successful, else 1
 */
static int
getCommandLineArguments(int argc, char * argv[], CommandLineArguments * const arguments)
{
	int opt;

	opterr = 0;

	while ((opt = getopt(argc, argv, ":d:D:e:E:a:A:t:h")) != EOF)
	{
		switch (opt)
		{
		case 'd':
			arguments->heaveDisplacementFilePath = optarg;
			break;
		case 'D':
			arguments->heaveMeasurementUncertainty = atof(optarg);
			break;
		case 'e':
			arguments->waveElevationFilePath = optarg;
			break;
		case 'E':
			arguments->waveElevationUncertainty = atof(optarg);
			break;
		case 'a':
			arguments->heaveAccelerationFilePath = optarg;
			break;
		case 'A':
			arguments->accelerometerResolution = atof(optarg);
			break;
		case 't':
			arguments->timestep = atof(optarg);
			if (arguments->timestep == 0.0)
			{
				printf("Error: invalid timestep value: %f\n", arguments->timestep);
				printUsage();
				return 1;
			}
			break;
		case 'h':
			printUsage();
			exit(0);
		case ':':
			printf("Error: option -%c is missing a required argument\n", optopt);
			return 1;
		case '?':
			printf("Error: invalid option: -%c\n", optopt);
			printUsage();
			return 1;
		}
	}

	return 0;
}

int
main(int argc, char * argv[])
{
	Buffer RAOBuffer = {
		.heapPointer = NULL,
		.size = 0,
	};
	Buffer waveSpectrumEstimateBuffer = {
		.heapPointer = NULL,
		.size = 0,
	};
	int                  returnValue = 0;
	CommandLineArguments arguments = {
		.heaveDisplacementFilePath = "testingHeave.csv",
		.heaveMeasurementUncertainty = 0.1,
		.waveElevationFilePath = "testingWaveElevation.csv",
		.waveElevationUncertainty = 0.1,
		.heaveAccelerationFilePath = "oceanHeaveAcceleration.csv",
		.accelerometerResolution = 0.1,
		.timestep = 0.1,
	};

	if (getCommandLineArguments(argc, argv, &arguments))
	{
		returnValue = 1;
		goto EXIT_PROGRAM;
	}

	if (characteriseRAO(
		    &RAOBuffer,
		    arguments.heaveDisplacementFilePath,
		    arguments.waveElevationFilePath,
		    arguments.heaveMeasurementUncertainty,
		    arguments.waveElevationUncertainty,
		    arguments.timestep))
	{
		returnValue = 1;
		goto EXIT_PROGRAM;
	}

	if (estimateWaveSpectrum(
		    &waveSpectrumEstimateBuffer,
		    &RAOBuffer,
		    arguments.heaveAccelerationFilePath,
		    arguments.accelerometerResolution,
		    arguments.timestep))
	{
		returnValue = 1;
		goto EXIT_PROGRAM;
	}

	if (waveSpectrumEstimateBuffer.heapPointer == NULL)
	{
		returnValue = 1;
		goto EXIT_PROGRAM;
	}
	else
	{
		const size_t maximumIndex = waveSpectrumEstimateBuffer.size / 2;
		size_t       arrayInterval = 1;

		if (maximumIndex > kMaximumPrintLinesInOutput)
		{
			arrayInterval = maximumIndex / (kMaximumPrintLinesInOutput - 1);
		}

		printf("Wave spectrum: (frequency, wave energy spectral density)\n");
		for (size_t i = 0; i <= waveSpectrumEstimateBuffer.size / 2; i += arrayInterval)
		{
			const float deltaF = 1 / (arguments.timestep * RAOBuffer.size);
			const float frequency = deltaF * i;
			printf("%f Hz, %f\n", frequency, waveSpectrumEstimateBuffer.heapPointer[i]);
		}
	}

EXIT_PROGRAM:
	freeHeapBuffer(&RAOBuffer);
	freeHeapBuffer(&waveSpectrumEstimateBuffer);
	return returnValue;
}
