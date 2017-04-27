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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bignbr.h"
#include "expression.h"
#include "highlevel.h"
#include <math.h>
#define MAX_SIEVE 65536
#define SUBT 12
extern int lang;
static int primediv[256];
static int primeexp[256];
static limb number[MAX_LEN];
static limb origNbr[MAX_LEN];
static limb p[MAX_LEN];
extern limb q[MAX_LEN];
extern limb Mult1[MAX_LEN];
static limb Mult2[MAX_LEN];
extern limb Mult3[MAX_LEN];
extern limb Mult4[MAX_LEN];
static limb SquareMult1[MAX_LEN];
static limb SquareMult2[MAX_LEN];
static limb SquareMult3[MAX_LEN];
static limb SquareMult4[MAX_LEN];
static limb Sum[MAX_LEN];
static int iMult3, iMult4;
static int Mult1Len, Mult2Len, Mult3Len, Mult4Len, power4;
static limb result[MAX_LEN];
static int nbrLimbs, origNbrLimbs;
static int sieve[MAX_SIEVE];
static int TerminateThread, sum, sumHi, sumLo;
static int nbrModExp;
extern limb TestNbr[MAX_LEN];
extern limb MontgomeryMultR1[MAX_LEN];
char texto[500];
BigInteger ExpressionResult;
void DivideBigNbrByMaxPowerOf2(int *pShRight, limb *number, int *pNbrLimbs);
int checkMinusOne(limb *value, int nbrLimbs);
#ifdef __EMSCRIPTEN__
void contfracText(char *input, int groupLen);
void fcubesText(char *input, int groupLen);
#endif

      // Find power of 4 that divides the number.
      // output: pNbrLimbs = pointer to number of limbs
      //         pPower4 = pointer to power of 4.
static void DivideBigNbrByMaxPowerOf4(int *pPower4, limb *value, int *pNbrLimbs)
{
  int powerOf4;
  int powerOf2 = 0;
  int numLimbs = *pNbrLimbs;
  int index, index2, mask, power2gr, shRg;
  limb carry;
    // Start from least significant limb (number zero).
  for (index = 0; index < numLimbs; index++)
  {
    if ((value + index)->x != 0)
    {
      break;
    }
    powerOf2 += BITS_PER_GROUP;
  }
  for (mask = 0x1; mask <= MAX_VALUE_LIMB; mask *= 2)
  {
    if (((value + index)->x & mask) != 0)
    {
      break;
    }
    powerOf2++;
  }
  powerOf4 = powerOf2 >> 1;
    // Divide value by this power.
  power2gr = powerOf2 % (2*BITS_PER_GROUP);
  shRg = (power2gr & (-2)) % BITS_PER_GROUP; // Shift right bit counter
  if (power2gr == BITS_PER_GROUP)
  {
    index--;
  }
  carry.x = 0;
  mask = (1 << shRg) - 1;
  for (index2 = numLimbs-1; index2 >= index; index2--)
  {
    carry.x = (carry.x << BITS_PER_GROUP) + (value+index2)->x;
    (value + index2)->x = carry.x >> shRg;
    carry.x &= mask;
  }
  if (index != 0)
  {
    memmove(value, value+index, (numLimbs-index)*sizeof(limb));
  }
  if ((value+numLimbs-1)->x != 0)
  {
    *pNbrLimbs = numLimbs - index;
  }
  else
  {
    *pNbrLimbs = numLimbs - index - 1;
  }
  *pPower4 = powerOf4;
}

 // If Mult1 < Mult2, exchange both numbers.
static void SortBigNbrs(limb *mult1, int *mult1Len, limb *mult2, int *mult2Len)
{
  limb tmp;
  int index;
  limb *ptr1, *ptr2;
  int len = *mult1Len;
  if (len > *mult2Len)
  {
    return;    // mult1 > mult2, so nothing to do.
  }
  if (len == *mult2Len)
  {
    ptr1 = &mult1[len-1];
    ptr2 = &mult2[len-1];
    for (index = *mult1Len - 1; index >= 0; index--)
    {
      if (ptr1->x > ptr2->x)
      {
        return;    // mult1 > mult2, so nothing to do.
      }
      if (ptr1->x < ptr2->x)
      {
        break;     // mult1 < mult2, so exchange them.
      }
      ptr1--;
      ptr2--;
    }
  }
    // Exchange lengths.
  len = *mult2Len;
  *mult2Len = *mult1Len;
  *mult1Len = len;
    // Exchange bytes that compose the numbers.
  ptr1 = mult1;
  ptr2 = mult2;
  for (index = 0; index < len; index++)
  {
    tmp.x = ptr1->x;
    ptr1->x = ptr2->x;
    ptr2->x = tmp.x;
    ptr1++;
    ptr2++;
  }
}

 // Variable to split in up to four squares: number
int fsquares(void)
{
  int sqrtFound, r, tmp;
  int i, j, numberMod8, Computing3Squares, nbrDivisors;
  int index, nbrLimbsP, nbrLimbsQ, shRight, shRightMult3, count;
  int divisor, base, idx, nbrLimbsSq, nbrLimbsBrillhart;
  limb carry, carry2;
  nbrLimbs = origNbrLimbs;
  memcpy(number, origNbr, nbrLimbs*sizeof(limb));
  squareRoot(number, Mult1, nbrLimbs, &Mult1Len);
  multiply(Mult1, Mult1, result, Mult1Len, NULL);
  if (memcmp(result, number, sizeof(limb)*nbrLimbs) == 0)
  {          // number is a perfect square.
    Mult2[0].x = 0;
    Mult2Len = 1;
    Mult3[0].x = 0;
    Mult3Len = 1;
    Mult4[0].x = 0;
    Mult4Len = 1;
    power4 = 0;
  }
  else
  {          // number is not a perfect square.
    nbrModExp = 0;
    for (i = 0; i < MAX_SIEVE / 2; i++)
    {
      sieve[i] = 0;
    }
    for (i = 3; i*i < MAX_SIEVE / 2; i += 2)
    {
      j = i*i - 3;
      j = (j % 2 == 0) ? j / 2 : (j + i) / 2;
      for (; j < MAX_SIEVE / 2; j += i)
      {
        sieve[j] = -1;             // Indicate number is composite.
      }
    }
    DivideBigNbrByMaxPowerOf4(&power4, number, &nbrLimbs);
    Mult1Len = Mult2Len = 1;
    if (nbrLimbs == 1 && number[0].x < 4)
    {
      iMult3 = iMult4 = 0;
      switch (number[0].x)
      {
      case 3:
        iMult3 = 1;
        /* no break */
      case 2:
        Mult2[0].x = 1;
        /* no break */
      case 1:
        Mult1[0].x = 1;
      }
    }
    else
    {     // Number greater than 3.
      numberMod8 = number[0].x & 7;              // modulus mod 8

      // Fill sieve array

      for (i = 0; i < MAX_SIEVE / 2; i++)
      {
        if (sieve[i] >= 0)                        // If prime...
        {
          limb Divid;
          limb Rem;
          int Q;
          Rem.x = 0;
          Q = 2 * i + 3;                              // Prime
          for (j = nbrLimbs - 1; j >= 0; j--)
          {
            Divid.x = number[j].x + (Rem.x << BITS_PER_GROUP);
            Rem.x = Divid.x % Q;
          }
          sieve[i] = (int)Rem.x;
        }
      }

      if (numberMod8 != 7)
      {              // n!=7 (mod 8) => Sum of three squares
        Computing3Squares = 1;
        iMult4 = 0;
        iMult3 = -1;
      }
      else
      {              // n==7 (mod 8) => Sum of four squares
        iMult3 = -1;
        iMult4 = 0;
        Computing3Squares = 0;
      }
      // If number is a sum of three squares, subtract a small square.
      // If number is a sum of four squares, subtract two small squares.
      // If the result is not the product of a power of 2, small primes
      // of the form 4k+1 and squares of primes of the form (4k+3)^2
      // and a (big) prime, try with other squares.
    compute_squares_loop:
      for (;;)
      {
        if (TerminateThread != 0)
        {
          return 2;
        }
        if (Computing3Squares == 0 && iMult3 >= iMult4)
        {
          iMult3 = 1;
          iMult4++;
        }
        else
        {
          iMult3++;
          if (iMult3 == 67)
          {
            iMult4 = 0;
          }
        }
        sum = iMult3 * iMult3 + iMult4 * iMult4;
        sumHi = sum >> BITS_PER_GROUP;
        sumLo = sum & MAX_VALUE_LIMB;
        carry.x = number[0].x - sumLo;
        p[0].x = carry.x & MAX_VALUE_LIMB;
        carry.x >>= BITS_PER_GROUP;
        if (nbrLimbs > 1)
        {
          carry.x += number[1].x - sumHi;
          p[1].x = carry.x & MAX_VALUE_LIMB;
          carry.x >>= BITS_PER_GROUP;
          for (index = 2; index < nbrLimbs; index++)
          {
            carry.x += number[index].x;
            p[index].x = carry.x & MAX_VALUE_LIMB;
            carry.x >>= BITS_PER_GROUP;
          }
        }
        // p should be the product of power of 2,
        // powers of small primes of form 4k+1 and
        // powers of squares of small primes of form 4k+3.
        nbrLimbsP = nbrLimbs;
        DivideBigNbrByMaxPowerOf2(&shRight, p, &nbrLimbsP);
        if ((p[0].x & 0x03) != 0x01)
        {
          continue;         /* p is not congruent to 1 mod 4 */
        }

        // Now compute modulus over the product of all odd divisors
        // less than maxsieve.
        if (shRight != 0)
        {
          primediv[0] = 2;                      // Store divisor.
          primeexp[0] = shRight;                // Store exponent.
          nbrDivisors = 1;
        }
        else
        {
          nbrDivisors = 0;
        }
        divisor = 3;
        for (i = 0; i < MAX_SIEVE / 2; i++)
        {
          if (sieve[i] >= 0 && (sieve[i] - sum) % divisor == 0)
          {                          // Divisor found.
            primediv[nbrDivisors] = divisor;   // Store divisor.
            primeexp[nbrDivisors] = 0;         // Store exponent.
            for (;;)
            {
              // Compute the remainder of p and divisor.
              carry.x = 0;
              for (index = nbrLimbsP - 1; index >= 0; index--)
              {
                carry.x = ((carry.x << BITS_PER_GROUP) + p[index].x) % divisor;
              }
              if (carry.x != 0)
              {     // Number is not a multiple of "divisor".
                break;
              }
              carry.x = 0;
              // Divide by divisor.
              for (index = nbrLimbsP - 1; index >= 0; index--)
              {
                carry.x = (carry.x << BITS_PER_GROUP) + p[index].x;
                p[index].x = carry.x / divisor;
                carry.x %= divisor;
              }
              if (p[nbrLimbsP - 1].x == 0)
              {
                nbrLimbsP--;
              }
              primeexp[nbrDivisors]++;       // Increment exponent.
            }
            if ((divisor & 0x03) == 3 && (primeexp[nbrDivisors] & 1) != 0)
            {                   // Divisor of the form 4k+3 not square.
              goto compute_squares_loop;  // Discard this value.
            }
            nbrDivisors++;
          }
          divisor += 2;
        }

        if (p[0].x == 1 && nbrLimbsP == 1)
        {         // number is the product of only small primes 4k+1 and
          // squares of primes 4k+3.
          Mult1[0].x = 1;
          Mult2[0].x = 0;
          nbrLimbs = 1;
        }
        else
        {
          // At this moment p should be prime, otherwise we must try another number.
          p[nbrLimbsP].x = 0;
          memcpy(q, p, (nbrLimbsP+1)*sizeof(p[0]));
          nbrLimbsQ = nbrLimbsP;
          q[0].x--;                     // q = p - 1 (p is odd, so there is no carry).
          memcpy(Mult3, q, (nbrLimbsQ+1)*sizeof(q[0]));
          Mult3Len = nbrLimbsP;
          DivideBigNbrByMaxPowerOf2(&shRightMult3, Mult3, &Mult3Len);
          base = 1;
          memcpy(TestNbr, p, (nbrLimbsP+1)*sizeof(p[0]));
          GetMontgomeryParms(nbrLimbsP);
          sqrtFound = 0;
          do
          {                 // Compute Mult1 = sqrt(-1) (mod p).
            base++;
            modPowBaseInt(base, Mult3, Mult3Len, Mult1); // Mult1 = base^Mult3.
            nbrModExp++;   // Increment number of modular exponentiations.    
            for (i = 0; i < shRightMult3; i++)
            {              // Loop that squares number.
              modmult(Mult1, Mult1, Mult4);
              if (checkMinusOne(Mult4, nbrLimbsP) != 0)
              {
                sqrtFound = 1;
                break;      // Mult1^2 = -1 (mod p), so exit loop.
              }
              memcpy(Mult1, Mult4, nbrLimbsP*sizeof(limb));
            }
            // If power (Mult4) is 1, that means that number is at least PRP,
            // so continue loop trying to find square root of -1.
          } while (memcmp(Mult4, MontgomeryMultR1, nbrLimbsP*sizeof(limb)) == 0);
          if (sqrtFound == 0)
          {            // Cannot find sqrt(-1) (mod p), go to next candidate.
            continue;
          }
          // Convert Mult1 from Montgomery notation to standard number
          // by multiplying by 1 in Montgomery notation.
          memset(Mult2, 0, nbrLimbsP*sizeof(limb));
          Mult2[0].x = 1;
          modmult(Mult2, Mult1, Mult1);  // Get sqrt(-1) mod p.
          // Find the sum of two squares that equals this PRP using
          // Brillhart's method detailed in
          // http://www.ams.org/journals/mcom/1972-26-120/S0025-5718-1972-0314745-6/S0025-5718-1972-0314745-6.pdf
          // Starting with x <- p, y <- sqrt(-1) (mod p) perform
          // z <- x mod y, x <- y, y <- z.
          // The reduction continues until the first time that x < sqrt(p).
          // In that case: x^2 + y^2 = p.

          // Result is stored in Mult1 and Mult2.
          squareRoot(TestNbr, Sum, nbrLimbsP, &nbrLimbsSq);
          memcpy(Mult2, TestNbr, nbrLimbsP*sizeof(limb));
          nbrLimbsBrillhart = nbrLimbsP;
          for (;;)
          {
            AdjustModN(Mult2, Mult1, nbrLimbsBrillhart);
            memcpy(Mult3, Mult2, nbrLimbsBrillhart*sizeof(limb));
            memcpy(Mult2, Mult1, nbrLimbsBrillhart*sizeof(limb));
            memcpy(Mult1, Mult3, nbrLimbsBrillhart*sizeof(limb));
            if (Mult2[nbrLimbsBrillhart - 1].x == 0)
            {
              nbrLimbsBrillhart--;
            }
            if (nbrLimbsBrillhart > nbrLimbsSq)
            {
              continue;
            }
            if (nbrLimbsBrillhart < nbrLimbsSq)
            {
              break;
            }
            for (idx = nbrLimbsBrillhart - 1; idx >= 0; idx--)
            {
              if (Mult2[idx].x != Sum[idx].x)
              {
                break;
              }
            }
            if (idx < 0 || Mult2[idx].x < Sum[idx].x)
            {
              break;
            }
          }
          nbrLimbs = nbrLimbsBrillhart;
        }
        // Use the other divisors of modulus in order to get Mult1 and Mult2.

        for (i = 0; i < nbrDivisors; i++)
        {
          divisor = primediv[i];
          for (index = primeexp[i]-1; index > 0; index -= 2)
          {
            carry.x = 0;
            carry2.x = 0;
            for (count = 0; count < nbrLimbs; count++)
            {
              carry.x += Mult1[count].x * divisor;
              carry2.x += Mult2[count].x * divisor;
              Mult1[count].x = carry.x & MAX_VALUE_LIMB;
              carry.x >>= BITS_PER_GROUP;
              Mult2[count].x = carry2.x & MAX_VALUE_LIMB;
              carry2.x >>= BITS_PER_GROUP;
            }
            Mult1[count].x = carry.x;
            Mult2[count].x = carry2.x;
            if (Mult1[count].x != 0 || Mult2[count].x != 0)
            {
              nbrLimbs++;
            }
          }
          if (index == 0)
          {
            // Since the value primediv[i] is very low, it is faster to use
            // trial and error than the general method in order to find
            // the sum of two squares.
            j = 1;
            for (;;)
            {
              r = (int)sqrt((double)(divisor - j*j));
              if (r*r + j*j == divisor)
              {
                break;
              }
              j++;
            }
            carry.x = 0;
            carry2.x = 0;
            for (count = 0; count < nbrLimbs; count++)
            {
              carry.x += Mult1[count].x * j + Mult2[count].x * r;
              carry2.x += Mult1[count].x * r - Mult2[count].x * j;
              Mult1[count].x = carry.x & MAX_VALUE_LIMB;
              carry.x >>= BITS_PER_GROUP;
              Mult2[count].x = carry2.x & MAX_VALUE_LIMB;
              carry2.x >>= BITS_PER_GROUP;
            }
            Mult1[count].x = carry.x;
            Mult2[count].x = carry2.x;
            if (carry2.x < 0)
            {   // Since Mult2 is a difference of products, it can be
                // negative. In this case replace it by its absolute value.
              Mult2[count].x = carry2.x & MAX_VALUE_LIMB;
              carry2.x = 0;
              for (count = 0; count <= nbrLimbs; count++)
              {
                carry2.x -= Mult2[count].x;
                Mult2[count].x = carry2.x & MAX_VALUE_LIMB;
                carry2.x >>= BITS_PER_GROUP;
              }
            }
            if (Mult1[nbrLimbs].x != 0 || Mult2[nbrLimbs].x != 0)
            {
              nbrLimbs++;
            }
          }
        }            /* end for */
        break;
      }              /* end while */
    }
    // Shift left the number of bits that the original number was divided by 4.
    for (count = 0; count < power4; count++)
    {
      carry.x = 0;
      carry2.x = 0;
      for (index = 0; index < nbrLimbs; index++)
      {
        carry.x += Mult1[index].x * 2;
        carry2.x += Mult2[index].x * 2;
        Mult1[index].x = carry.x & MAX_VALUE_LIMB;
        carry.x >>= BITS_PER_GROUP;
        Mult2[index].x = carry2.x & MAX_VALUE_LIMB;
        carry2.x >>= BITS_PER_GROUP;
      }
      Mult1[index].x = carry.x;
      Mult2[index].x = carry2.x;
      if (Mult1[index].x != 0 || Mult2[index].x != 0)
      {
        nbrLimbs++;
      }
    }
    nbrLimbsP = 1;
    Mult3[0].x = iMult3;
    Mult4[0].x = iMult4;
    for (count = 0; count < power4; count++)
    {
      carry.x = 0;
      carry2.x = 0;
      for (index = 0; index < nbrLimbsP; index++)
      {
        carry.x += Mult3[index].x * 2;
        carry2.x += Mult4[index].x * 2;
        Mult3[index].x = carry.x & MAX_VALUE_LIMB;
        carry.x >>= BITS_PER_GROUP;
        Mult4[index].x = carry2.x & MAX_VALUE_LIMB;
        carry2.x >>= BITS_PER_GROUP;
      }
      Mult3[index].x = carry.x;
      Mult4[index].x = carry2.x;
      if (Mult3[index].x != 0 || Mult4[index].x != 0)
      {
        nbrLimbsP++;
      }
    }
    Mult1Len = Mult2Len = nbrLimbs;
    while (Mult1[Mult1Len-1].x == 0 && Mult1Len > 1)
    {
      Mult1Len--;
    }
    while (Mult2[Mult2Len-1].x == 0 && Mult2Len > 1)
    {
      Mult2Len--;
    }
    Mult3Len = Mult4Len = nbrLimbsP;
    while (Mult3[Mult3Len-1].x == 0 && Mult3Len > 1)
    {
      Mult3Len--;
    }
    while (Mult1[Mult4Len-1].x == 0 && Mult4Len > 1)
    {
      Mult4Len--;
    }
    // Sort squares
    SortBigNbrs(Mult1, &Mult1Len, Mult2, &Mult2Len);
    SortBigNbrs(Mult1, &Mult1Len, Mult3, &Mult3Len);
    SortBigNbrs(Mult1, &Mult1Len, Mult4, &Mult4Len);
    SortBigNbrs(Mult2, &Mult2Len, Mult3, &Mult3Len);
    SortBigNbrs(Mult2, &Mult2Len, Mult4, &Mult4Len);
    SortBigNbrs(Mult3, &Mult3Len, Mult4, &Mult4Len);
  }
    // Validate result.
  multiply(Mult1, Mult1, SquareMult1, Mult1Len, &tmp);
  multiply(Mult2, Mult2, SquareMult2, Mult2Len, &tmp);
  memset(&SquareMult2[Mult2Len << 1], 0, ((Mult1Len - Mult2Len) << 1)*sizeof(limb));
  multiply(Mult3, Mult3, SquareMult3, Mult3Len, &tmp);
  memset(&SquareMult3[Mult3Len << 1], 0, ((Mult1Len - Mult3Len) << 1)*sizeof(limb));
  multiply(Mult4, Mult4, SquareMult4, Mult4Len, &tmp);
  memset(&SquareMult4[Mult4Len << 1], 0, ((Mult1Len - Mult4Len) << 1)*sizeof(limb));
  idx = Mult1Len * 2;
  carry.x = 0;
  for (index = 0; index < idx; index++)
  {
    carry.x += SquareMult1[index].x + SquareMult2[index].x +
    SquareMult3[index].x + SquareMult4[index].x;
    SquareMult1[index].x = carry.x & MAX_VALUE_LIMB;
    carry.x >>= BITS_PER_GROUP;
  }
  if (carry.x != 0)
  {
    SquareMult1[index].x = carry.x;
    idx++;
  }
  if (SquareMult1[idx - 1].x == 0)
  {
    idx--;
  }
  if (idx != origNbrLimbs)
  {           // Invalid length.
    return 1;
  }
  for (index = 0; index < idx; index++)
  {
    if (SquareMult1[index].x != origNbr[index].x)
    {
      return 1;
    }
  }
  return 0;
}

void fsquaresText(char *input, int groupLength)
{
  char *square = "<span class=\"bigger\">²</span>";
  enum eExprErr rc;
  char *ptrOutput = output;
  rc = ComputeExpression(input, 1, &ExpressionResult);
  if (rc != EXPR_OK)
  {
    textError(ptrOutput, rc);
    return;
  }
  if (ExpressionResult.sign == SIGN_NEGATIVE)
  {
    textError(ptrOutput, EXPR_NUMBER_TOO_LOW);
    return;
  }
  origNbrLimbs = ExpressionResult.nbrLimbs;
  memcpy(origNbr, ExpressionResult.limbs, origNbrLimbs*sizeof(limb));
  switch (fsquares())
  {
  case 1:
    strcpy(ptrOutput, (lang==0?"<p>Internal error!\n\nPlease send the number to the author of the applet.</p>":
      "<p>¡Error interno!\n\nPor favor envíe este número al autor del applet.</p>"));
    return;
  case 2:
    strcpy(ptrOutput, (lang==0?"<p>User stopped the calculation":"</p>El usuario detuvo el cálculo"));
    return;
  }
  // Show the number to be decomposed into sum of cubes.
  strcpy(ptrOutput, "<p><var>n</var> = ");
  ptrOutput += strlen(ptrOutput);
  BigInteger2Dec(&ExpressionResult, ptrOutput, groupLength);
  ptrOutput += strlen(ptrOutput);
  // Show whether the number is a sum of 1, 2, 3 or 4 squares.
  strcpy(ptrOutput, "</p><p><var>n</var> = <var>a</var>");
  ptrOutput += strlen(ptrOutput);
  strcpy(ptrOutput, square);
  ptrOutput += strlen(ptrOutput);
  if (Mult2Len != 1 || Mult2[0].x != 0)
  {
    strcpy(ptrOutput, " + <var>b</var>");
    ptrOutput += strlen(ptrOutput);
    strcpy(ptrOutput, square);
    ptrOutput += strlen(ptrOutput);
  }
  if (Mult3Len != 1 || Mult3[0].x != 0)
  {
    strcpy(ptrOutput, " + <var>c</var>");
    ptrOutput += strlen(ptrOutput);
    strcpy(ptrOutput, square);
    ptrOutput += strlen(ptrOutput);
  }
  if (Mult4Len != 1 || Mult4[0].x != 0)
  {
    strcpy(ptrOutput, " + <var>d</var>");
    ptrOutput += strlen(ptrOutput);
    strcpy(ptrOutput, square);
    ptrOutput += strlen(ptrOutput);
  }
  strcpy(ptrOutput, "</p><p><span class=\"offscr\">");
  ptrOutput += strlen(ptrOutput);
  strcpy(ptrOutput, lang ? " donde: </span>" : " where: </span>");
  ptrOutput += strlen(ptrOutput);
  // Show the decomposition.
  strcpy(ptrOutput, "<p><var>a</var> = ");
  ptrOutput += strlen(ptrOutput);
  Bin2Dec(Mult1, ptrOutput, Mult1Len, groupLength);
  ptrOutput += strlen(ptrOutput);
  if (Mult2Len != 1 || Mult2[0].x != 0)
  {
    strcpy(ptrOutput, "</p><p><var>b</var> = ");
    ptrOutput += strlen(ptrOutput);
    Bin2Dec(Mult2, ptrOutput, Mult2Len, groupLength);
    ptrOutput += strlen(ptrOutput);
  }
  if (Mult3Len != 1 || Mult3[0].x != 0)
  {
    strcpy(ptrOutput, "</p><p><var>c</var> = ");
    ptrOutput += strlen(ptrOutput);
    Bin2Dec(Mult3, ptrOutput, Mult3Len, groupLength);
    ptrOutput += strlen(ptrOutput);
  }
  if (Mult4Len != 1 || Mult4[0].x != 0)
  {
    strcpy(ptrOutput, "</p><p><var>d</var> = ");
    ptrOutput += strlen(ptrOutput);
    Bin2Dec(Mult4, ptrOutput, Mult4Len, groupLength);
    ptrOutput += strlen(ptrOutput);
  }
  strcpy(ptrOutput, (lang?"</p><p>" COPYRIGHT_SPANISH "</p>":
                          "</p><p>" COPYRIGHT_ENGLISH "</p>"));
}

#ifdef __EMSCRIPTEN__
void databack(char *data);
void doWork(char* data, int size)
{
  int groupLen = 0;
  char *ptrData = data;
  while (*ptrData != ',')
  {
    groupLen = groupLen * 10 + (*ptrData++ - '0');
  }
  ptrData++;             // Skip comma.
  lang = *ptrData & 1;
  switch ((*ptrData - '0') >> 1)
  {
  case 0:
    fsquaresText(ptrData+2, groupLen);
    break;
  case 1:
    fcubesText(ptrData+2, groupLen);
    break;
  case 2:
    contfracText(ptrData+2, groupLen);
    break;
  }
  databack(output);
}
#endif
