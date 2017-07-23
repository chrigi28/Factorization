#ifndef _HELPER_H
#define _HELPER_H
#include "mpi.h"
#include "factor.h"
#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

char savePath[1000];
enum Messages {
	GET_JOB = 0,
	SEND_JOB,
	SEND_RESULT,
	SEND_RDY_FACTOR,
	JOBTOFACTOR,
	START_FACTORING,
	REGISTER_FOR_EC,
	SEND_EC,
	NO_OF_PSTFACTORS,
	SEND_MULTIPLICITY,
	SEND_UPPERBOUND, //10
	SEND_PTRFACTOR,
	CHECK_FACTOR,
	GD_LENGTH,
	TEST_GD,
	NUMBER_LENGTH,
	INTERRUPT_EC,
	GET_FACTOR_DATA,
	FACTORING_DONE,
	LOAD_EC,
	LOGOUT,
	SEND_CHAR
};
extern char * STATENAMES[];

enum eEcmResult{
	FACTOR_NOT_FOUND = 0,
	FACTOR_FOUND,
	CHANGE_TO_SIQS,
	GET_NEXT_EC,
	FACTOR_FOUND_BY_PARALLEL, // cho
	CLOSE_PROZESS
};

void sendBigInteger(BigInteger *number,int dest,int source);
void receiveBigInteger(BigInteger *number, int source,int dest);
void sendPstFactors(struct sFactors *pstFactors,int dest,int source);
void receivePstFactors(struct sFactors *pstFactors,int source,int dest);
void printPstFactors(struct sFactors *pstFactors, int source);
void writeFactorToDisk(struct sFactors *pstFactors);
void loadCurrentEc(int *EC);
void saveCurrentEc(int EC);
void saveFactorizationToText(struct sFactors *pstFactors,char *path);
void readFactorFromDisk(struct sFactors *pstFactors,char* pathToFolder);
void logPstFactors(struct sFactors *pstFactors);
void ecmFrontText(char *tofactorText, int doFactorization, char *knownFactors,int world_rank);
void showFactors(BigInteger *N,struct sFactors *pstFactors,int world_rank);
void cho_Waitany(MPI_Status *status);
void cho_WaitSpecific(MPI_Status *status,int tag);
void setSavePoint(char *tofactor);
#endif
