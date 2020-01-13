/*
 * Deviation.cpp
 *
 *  Created on: 13 Jan 2020
 *      Author: David
 */

#include "Deviation.h"
#include <cmath>

Deviation::Deviation() : mean(0.0), deviationFromMean(0.0)
{
}

void Deviation::Set(float sumOfSquares, float sum, size_t numPoints)
{
	// Use: average sum of squares = average sum of squares of difference from mean + mean squared
	if (numPoints == 0)
	{
		mean = deviationFromMean = 0.0;
	}
	else
	{
		mean = sum/numPoints;
		deviationFromMean = sqrtf(sumOfSquares/numPoints - mean * mean);
	}
}

// End
