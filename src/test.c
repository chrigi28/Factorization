#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bignbr.h"
#include "highlevel.h"
#include "factor.h"
#define DEBUG_CODE 13
void dilogText(char *baseText, char *powerText, char *modText, int groupLen);
void gaussianText(char *valueText, int doFactorization);
void ecmFrontText(char *tofactorText, int doFactorization, char *knownFactors);
int Factor1[] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00 };
int Factor2[] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00 };
int Factor3[] = { 29504, 29490, 19798, 633, 181, 0, 0, 0, 0, 0 };
int Factor4[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int Factor5[] = { 32767, 32767, 32767, 32767, 0, 0, 0, 0 };
int Factor6[] = { 32767, 32767, 32767, 32767, 0, 0, 0, 0 };
int factors[5000];
struct sFactors astFactors[1000];
extern int karatCtr, multCtr;
extern int number[MAX_LEN];
extern int nbrLimbs;
extern int lang, groupLen;
extern limb TestNbr[MAX_LEN];
char expr[] = "123456789012345";
extern char *output, batch;
int Product[32];
char input[10000];
BigInteger dividend, divisor, quotient;
int main2(int argc, char *argv[])
{ printf("\n cho here i am  start program <\n\n " );
  int len, i;
  output = (char *)malloc(10000000);
#if DEBUG_CODE == 1
{ printf("\n debug == 1 assert no \n\n " );
  fsquaresText(argv[1], 6);
  printf("%s\n", output);
  printf("multiplication count: %d, Karatsuba count: %d", multCtr, karatCtr);
#elif DEBUG_CODE == 13

  batch = 0;
  lang = 0;

  
  
  if (argc == 3)
  {
    ecmFrontText(argv[1], 1, argv[2]);
    printf("%s\n", output);
  }
  else if (argc == 2)
  {
	  printf("\n argc 2  assert no \n\n " );
    char *ptrKnownFactors = strchr(argv[1], '=');
    if (ptrKnownFactors)
    {                          // There is equal sign.
      *ptrKnownFactors = 0;    // Replace equal sign by string terminator.
      ptrKnownFactors++;
    }
    ecmFrontText(argv[1], 1, ptrKnownFactors);
    printf("%s\n", output);
  }
  else
  {
    //printf("value [known factors]\n");
    return 0;
  }
#endif
	char str2[10];
  //scanf("%s", str2);
  return 0;
}
