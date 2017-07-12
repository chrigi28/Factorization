#include "helper.h"
#include <stdlib.h>
#include <stdio.h>

char * STATENAMES[]={
	"GET_JOB\0",
	"SEND_JOB\0",
	"SEND_RESULT\0",
	"SEND_RDY_FACTOR\0",
	"JOBTOFACTOR\0",
	"START_FACTORING\0",
	"REGISTER_FOR_EC\0",
	"SEND_EC\0",
	"NO_OF_PSTFACTORS\0",
	"SEND_MULTIPLICITY\0",
	"SEND_UPPERBOUND\0",
	"SEND_PTRFACTOR\0",
	"CHECK_FACTOR\0",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
};
void sendBigInteger(BigInteger *number,int dest){
	MPI_Send( &(number->nbrLimbs), 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
	MPI_Send( &(number->limbs), number->nbrLimbs, MPI_INT, dest, 0, MPI_COMM_WORLD);
	MPI_Send( &(number->sign), 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
}
void receiveBigInteger(BigInteger *number, int source){
	MPI_Recv( &(number->nbrLimbs), 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv( &(number->limbs), number->nbrLimbs, MPI_INT,source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv( &(number->sign), 1,MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

}
/*
 sFactors pstFactors[1000]
 struct sFactors{
  int *ptrFactor;
  int multiplicity;
  int upperBound;
};

 */
void printPstFactors(struct sFactors *pstFactors){
	for(int i = 0; i<pstFactors[0].multiplicity;i++){
		printf("multiplicity from pstFactors[%d] is: %d\n",i,pstFactors[i].multiplicity);
		printf("upperbound from pstFactors[%d] is: %d\n",i,pstFactors[i].multiplicity);
		printf("SIZE of ptrFactor is %i \n",pstFactors[i].ptrFactor[0]);
		for(int j = 0;j<pstFactors[i].ptrFactor[0];j++){
			printf("data from pstFactors[%d].ptrFactor[%d] = %d\n",i,j,pstFactors[i].ptrFactor[j]);
		}
	}
	logPstFactors(&pstFactors[0]);
}

void logPstFactors(struct sFactors *pstFactors){
	FILE *fp;
	fp = fopen("logFile.log", "a");
	for(int i = 0; i<pstFactors[0].multiplicity;i++){
		fprintf(fp,"multiplicity from pstFactors[%d] is: %d\n",i,pstFactors[i].multiplicity);
		fprintf(fp,"upperbound from pstFactors[%d] is: %d\n",i,pstFactors[i].multiplicity);
		fprintf(fp,"SIZE of ptrFactor is %i \n",pstFactors[i].ptrFactor[0]);
		for(int j = 0;j<pstFactors[i].ptrFactor[0];j++){
			fprintf(fp,"data from pstFactors[%d].ptrFactor[%d] = %d\n",i,j,pstFactors[i].ptrFactor[j]);
		}
	}
	fclose(fp);

}

void sendPstFactors(struct sFactors *pstFactors,int dest){
	int NumberOfElements = pstFactors->multiplicity;


	MPI_Send( &NumberOfElements,1,MPI_INT,dest,NO_OF_PSTFACTORS,MPI_COMM_WORLD);
//	MPI_Send( &(pstFactors->upperBound),1,MPI_INT,dest,SEND_UPPERBOUND,MPI_COMM_WORLD);
//	int i = 0;
//	struct sFactors *pstFactor = pstFactors+1;
//	for(int i = 0; i<20;i++){
//		printf("%d\n",pstFactor->ptrFactor[i]);
//	}
//	printf("\n size of pstfactors is %d\n",pstFactors->multiplicity);
	for(int i = 0;i<pstFactors[0].multiplicity;i++){
		int numberLength = pstFactors[i].ptrFactor[0];
//		printf("sending %d data\n",numberLength);
//		for(int i = 0;i<numberLength;i++)
//			printf("send data: %d",*((pstFactor->ptrFactor)+i));
		MPI_Send( &(pstFactors[i].multiplicity), 1, MPI_INT, dest, SEND_MULTIPLICITY, MPI_COMM_WORLD);
		MPI_Send( &(pstFactors[i].upperBound), 1, MPI_INT, dest, SEND_UPPERBOUND, MPI_COMM_WORLD);
		MPI_Send( &(pstFactors[i].ptrFactor[0]), numberLength , MPI_INT, dest, SEND_PTRFACTOR, MPI_COMM_WORLD);

//		printf("\ni is %d max is %d\n",i+1,pstFactors->multiplicity);

	}
}

void receivePstFactors(struct sFactors *pstFactors,int source){
	int numberOfElements;
	MPI_Recv( &numberOfElements,1,MPI_INT,source,NO_OF_PSTFACTORS,MPI_COMM_WORLD,MPI_STATUSES_IGNORE);
//	int i = numberOfElements;
//	pstFactors->multiplicity = numberOfElements;
//	int* ptrData;
//	pstFactors->ptrFactor =ptrData;
//	MPI_Recv( &(pstFactors->upperBound),1,MPI_INT,source,SEND_UPPERBOUND,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
//	struct sFactors *pstFactor = pstFactors+1;
	int count = 0;
	for(int i = 0;i<numberOfElements;i++){
		MPI_Recv( &(pstFactors[i].multiplicity), 1, MPI_INT, source, SEND_MULTIPLICITY, MPI_COMM_WORLD,MPI_STATUSES_IGNORE);
		MPI_Recv( &(pstFactors[i].upperBound), 1, MPI_INT, source, SEND_UPPERBOUND, MPI_COMM_WORLD,MPI_STATUSES_IGNORE);
		MPI_Status status;
		MPI_Probe(source,SEND_PTRFACTOR,MPI_COMM_WORLD,&status);

		MPI_Get_count(&status,MPI_INT,&count);
//		printf("count is %d\n",count);
		int temp[count];
//		printf("\naddress of ptrFactors[%d].ptrfactor[0] = %p\n",i,&temp[0]);
		pstFactors[i].ptrFactor =temp;
//		printf("waiting for data i: %d  max: %d\n",i,numberOfElements);
		MPI_Recv(temp, count , MPI_INT, source, SEND_PTRFACTOR, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		//&(pstFactor->ptrFactor)
//		printf("received: %d\n",temp[0]);
//		pstFactor++;
	}
//	pstFactor = pstFactors+1;
//	for(int i = 0; i<count;i++){
//		printf("received: %d\n",*(pstFactor->ptrFactor));
//	}
//	printf("\n receive end");
}

void showFactors(BigInteger *N,struct sFactors *pstFactors,int world_rank){
	char *ptrOutput;
	char output[3000000];
	ptrOutput = output;
	SendFactorizationToOutput(EXPR_OK,pstFactors,&ptrOutput,1);
	printf("factorisation is: \n%s",ptrOutput);
}

void cho_Waitany(MPI_Status *status){
	int noMess = 0;
	//printf("cho wait\n");
	while(noMess == 0){
		//usleep(100000);

		usleep(500000);
		MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&noMess,status);

	}
	//printf("received from %i, tag %i\n",status->MPI_SOURCE,status->MPI_TAG);
}
