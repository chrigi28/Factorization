/*
    This file is part of Alpertron Calculators.

    Copyright 2015 Dario Alejandro Alpern

    Alpertron Calculators is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Alpertron Calculators is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Alpertron Calculators.  If not, see <http://www.gnu.org/licenses/>.
    */

/* Perform Karatsuba multiplication in little endian order */

#include <string.h>
#include "bignbr.h"
#include <stdio.h>
#include <math.h>

#define KARATSUBA_CUTOFF 8
static limb arr[MAX_LEN];
static limb arrAux[MAX_LEN];
static int length;
static int karatLength;
static void Karatsuba(int idxFactor1, int length, int endIndex);
int multCtr, karatCtr;

void multiply(limb *factor1, limb *factor2, limb *result, int len, int *pResultLen)
{
	length = len;
    // Compute length of numbers for each recursion.
	if (length <= KARATSUBA_CUTOFF)
	{
		karatLength = length;
	}
	else
	{
    int div;

		div = 1;
		while (length > KARATSUBA_CUTOFF)
		{
			div *= 2;
			length = (length + 1) / 2;
		}
		length *= div;
		karatLength = length;
	}
  memset(arr, 0, 2 * length*sizeof(limb));
	memcpy(&arr[0], factor1, len*sizeof(limb));
	memcpy(&arr[length], factor2, len*sizeof(limb));
	Karatsuba(0, length, 2 * length);
  memcpy(result, &arr[2 * (karatLength - length)], 2 * length * sizeof(limb));
  if (pResultLen != NULL)
  {
    memcpy(result, &arr[2 * (karatLength - length)], 2 * length * sizeof(limb));
    if (arr[2 * (karatLength - length)-1].x == 0)
    {
      *pResultLen = length * 2 - 1;
    }
    else
    {
      *pResultLen = length * 2;
    }
  }
}

// The return value is the sign: true: negative.
// In result the absolute value of the difference is computed.
static int absSubtract(int idxMinuend, int idxSubtrahend,
	                     int idxResult, int length)
{
	int sign = 0;
	limb carry;
	int i;
	for (i = length-1; i>=0; i--)
	{
		if (arr[idxMinuend + i].x != arr[idxSubtrahend + i].x)
		{
			break;
		}
	}
	if (i>=0 && arr[idxMinuend + i].x < arr[idxSubtrahend + i].x)
	{
		sign = 1;
		i = idxMinuend;    // Exchange minuend and subtrahend.
		idxMinuend = idxSubtrahend;
		idxSubtrahend = i;
	}
  carry.x = 0;
  for (i = 0; i < length; i++)
  {
    carry.x += arr[idxMinuend + i].x - arr[idxSubtrahend + i].x;
    arr[idxResult + i].x = carry.x & MAX_VALUE_LIMB;
    carry.x >>= BITS_PER_GROUP;
  }
	return sign;
}

// Multiply two groups of length limbs. The first one starts at idxFactor1
// and the second one at idxFactor2. The 2*length limb result is stored
// starting at idxFactor1. Use arrAux as temporary storage.
// Accumulate products by result limb.
static void ClassicalMult(int idxFactor1, int idxFactor2, int length)
{
  int prodCol, fact1Col;
  limb *ptrFactor1, *ptrFactor2;
  double dRangeLimb = (double)LIMB_RANGE;
  double dInvRangeLimb = 1 / dRangeLimb;
  int low = 0;              // Low limb of sums of multiplications.
  double dAccumulator = 0;  // Approximation to the sum of multiplications.
  int factor1, factor2;
  multCtr++;
  for (prodCol = 0; prodCol < 2 * length - 1; prodCol++)
  {    // Process each limb of product (least to most significant limb).
    if (prodCol < length)
    {   // Processing first half (least significant) of product.
      ptrFactor2 = &arr[idxFactor2 + prodCol];
      ptrFactor1 = &arr[idxFactor1];
      fact1Col = prodCol;
    }
    else
    {  // Processing second half (most significant) of product.
      ptrFactor2 = &arr[idxFactor2 + length - 1];
      ptrFactor1 = &arr[idxFactor1 + prodCol - length + 1];
      fact1Col = 2 * (length - 1) - prodCol;
    }
    for (; fact1Col>=0; fact1Col--)
    {
      factor1 = (ptrFactor1++)->x;
      factor2 = (ptrFactor2--)->x;
      low += factor1 * factor2;
      dAccumulator += (double)factor1 * (double)factor2;
    }
    low &= MAX_VALUE_LIMB;    // Trim extra bits.
    arrAux[prodCol].x = low;
    // Subtract or add 0x20000000 so the multiplication by dVal is not nearly an integer.
    // In that case, there would be an error of +/- 1.
    if (low < HALF_INT_RANGE)
    {
      dAccumulator = floor(dAccumulator * dInvRangeLimb + 0.25);
    }
    else
    {
      dAccumulator = floor(dAccumulator * dInvRangeLimb - 0.25);
    }
    low = (unsigned int)(dAccumulator - floor(dAccumulator / dRangeLimb) * dRangeLimb);
  }
  arrAux[prodCol].x = low;
  memcpy(&arr[idxFactor1], &arrAux[0], 2 * length * sizeof(limb));
  return;
}

// Recursive Karatsuba function.
static void Karatsuba(int idxFactor1, int length, int endIndex)
{
	int idxFactor2 = idxFactor1 + length;
  int i;
  unsigned int carry1First, carry1Second, accum1Lo;
  unsigned int carry2Second, accum2Lo;
  limb *ptrResult, *ptrHigh, tmp;
	int middle;
	int sign;
	int halfLength;
	if (length <= KARATSUBA_CUTOFF)
	{
		// Check if one of the factors is equal to zero.
    ptrResult = &arr[idxFactor1];
		for (i = length; i > 0; i--)
		{
			if ((ptrResult++)->x != 0)
			{
				break;
			}
		}
		if (i > 0)
		{     // First factor is not zero. Check second.
      ptrResult = &arr[idxFactor2];
      for (i = length; i > 0; i--)
			{
        if ((ptrResult++)->x != 0)
				{
					break;
				}
			}
		}
		if (i==0)
		{    // One of the factors is equal to zero.
			for (i = length - 1; i >= 0; i--)
			{
				arr[idxFactor1 + i].x = arr[idxFactor2 + i].x = 0;
			}
			return;
		}
         // Below cutoff: perform standard classical multiplcation.
    ClassicalMult(idxFactor1, idxFactor2, length);
    return;
	}
	// Length > KARATSUBA_CUTOFF: Use Karatsuba multiplication.
  // It uses three half-length multiplications instead of four.
  //  x*y = (xH*b + xL)*(yH*b + yL)
  //  x*y = (b + 1)*(xH*yH*b + xL*yL) + (xH - xL)*(yL - yH)*b
  // The length of b is stored in variable halfLength.
  // Since the absolute values of (xH - xL) and (yL - yH) fit in
  // a single limb, there will be no overflow.

	// At this moment the order is: xL, xH, yL, yH.
	// Exchange high part of first factor with low part of 2nd factor.
  karatCtr++;
  halfLength = length >> 1;
  for (i = idxFactor1 + halfLength; i<idxFactor2; i++)
	{
		tmp.x = arr[i].x;
		arr[i].x = arr[i + halfLength].x;
		arr[i + halfLength].x = tmp.x;
	}
	// At this moment the order is: xL, yL, xH, yH.
	// Get absolute values of (xH-xL) and (yL-yH) and the signs.
	sign = absSubtract(idxFactor1, idxFactor2, endIndex, halfLength);
	sign ^= absSubtract(idxFactor2 + halfLength, idxFactor1 + halfLength,
		endIndex + halfLength, halfLength);
	middle = endIndex;
	endIndex += length;
	Karatsuba(idxFactor1, halfLength, endIndex); // Multiply both low parts.
	Karatsuba(idxFactor2, halfLength, endIndex); // Multiply both high parts.
	Karatsuba(middle, halfLength, endIndex);     // Multiply the differences.
     // Process all carries at the end.
     // Obtain (b+1)(xH*yH*b + xL*yL) = xH*yH*b^2 + (xL*yL+xH*yH)*b + xL*yL
     // The first and last terms are already in correct locations.
  ptrResult = &arr[idxFactor1+halfLength];
  carry1First = carry1Second = carry2Second = 0;
  for (i = halfLength; i > 0; i--)
  {
    // The sum of three ints overflows an unsigned int variable,
    // so two adds are required. Also carries must be separated in
    // order to avoid overflow:
    // 00000001 + 7FFFFFFF + 7FFFFFFF = FFFFFFFF
    accum1Lo = carry1First + ptrResult->x + (ptrResult + halfLength)->x;
    carry1First = accum1Lo >> BITS_PER_GROUP;
    accum2Lo = carry2Second + (accum1Lo & MAX_VALUE_LIMB) +
               (ptrResult - halfLength)->x;
    carry2Second = accum2Lo >> BITS_PER_GROUP;
    accum1Lo = carry1Second + (accum1Lo & MAX_VALUE_LIMB) +
               (ptrResult + length)->x;
    carry1Second = accum1Lo >> BITS_PER_GROUP;
    (ptrResult + halfLength)->x = accum1Lo & MAX_VALUE_LIMB;
    ptrResult->x = accum2Lo & MAX_VALUE_LIMB;
    ptrResult++;
  }
  (ptrResult + halfLength)->x += carry1First + carry1Second;
  ptrResult->x += carry1First + carry2Second;
  // Process carries.
  ptrResult = &arr[idxFactor1];
  carry1First = 0;
  for (i = 2*length; i > 0; i--)
  {
    carry1First += ptrResult->x;
    (ptrResult++)->x = carry1First & MAX_VALUE_LIMB;
    carry1First >>= BITS_PER_GROUP;
  }
  // Compute final product.
  ptrHigh = &arr[middle];
  ptrResult = &arr[idxFactor1 + halfLength];
  if (sign != 0)
  {            // (xH-xL) * (yL-yH) is negative.
    int borrow = 0;
    for (i = length; i > 0; i--)
    {
      borrow += ptrResult->x - (ptrHigh++)->x;
      (ptrResult++)->x = borrow & MAX_VALUE_LIMB;
      borrow >>= BITS_PER_GROUP;
    }
    for (i = halfLength; i > 0; i--)
    {
      borrow += ptrResult->x;
      (ptrResult++)->x = borrow & MAX_VALUE_LIMB;
      borrow >>= BITS_PER_GROUP;
    }
  }
  else
  {            // (xH-xL) * (yL-yH) is positive or zero.
    unsigned int carry = 0;
    for (i = length; i > 0; i--)
    {
      carry += (unsigned int)ptrResult->x + (unsigned int)(ptrHigh++)->x;
      (ptrResult++)->x = (int)(carry & MAX_VALUE_LIMB);
      carry >>= BITS_PER_GROUP;
    }
    for (i = halfLength; i > 0; i--)
    {
      carry += (unsigned int)ptrResult->x;
      (ptrResult++)->x = (int)(carry & MAX_VALUE_LIMB);
      carry >>= BITS_PER_GROUP;
    }
  }
}
