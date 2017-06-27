#ifndef _HELPER_H
#define _HELPER_H
#include "mpi.h"
#include "factor.h"


enum Messages {
	GET_JOB,
	SEND_JOB,
	SEND_RESULT,
	SEND_RDY_FACTOR,
	JOBTOFACTOR,
	START_FACTORING,
	REGISTER_FOR_EC,
	SEND_EC,
	NO_OF_PSTFACTORS,
	SEND_MULTIPLICITY,
	SEND_UPPERBOUND,
	SEND_PTRFACTOR
};

enum eEcmResult{
  FACTOR_NOT_FOUND = 0,
  FACTOR_FOUND,
  CHANGE_TO_SIQS,
  GET_NEXT_EC,
  FACTOR_FOUND_BY_PARALLEL, // cho
  CLOSE_PROZESS
};

void sendBigInteger(BigInteger *number,int dest);
void receiveBigInteger(BigInteger *number, int source);
void sendPstFactors(struct sFactors *pstFactors,int dest);
void receivePstFactors(struct sFactors *pstFactors,int source);

void ecmFrontText(char *tofactorText, int doFactorization, char *knownFactors);

#endif