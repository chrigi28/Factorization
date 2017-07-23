#include "helper.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


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
	"GD_LENGTH",
	"TEST_GD",
	"NUMBER_LENGTH",
	"INTERRUPT_EC",
	"GET_FACTOR_DATA",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
};
void sendBigInteger(BigInteger *number,int dest, int source){
	printf("sending bigInt from %d to %d\n",source,dest);
	MPI_Send( &(number->nbrLimbs), 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
	MPI_Send( &(number->limbs), number->nbrLimbs, MPI_INT, dest, 0, MPI_COMM_WORLD);
	MPI_Send( &(number->sign), 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
}
void receiveBigInteger(BigInteger *number, int source, int dest){
	printf("rank%d: receive bigInt from %d",dest,source);
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
void printPstFactors(struct sFactors *pstFactors, int source){
	printf("rank%d: PRINT PSTFACTORS\n",source);

	for(int i = 0; i<=pstFactors[0].multiplicity;i++){
		printf("multiplicity from pstFactors[%d] is: %d @%p\n",i,pstFactors[i].multiplicity,&(pstFactors[i].multiplicity));
		printf("upperbound from pstFactors[%d] is: %d @%p\n",i,pstFactors[i].upperBound,&(pstFactors[i].upperBound));
		printf("SIZE of pstFactors[%d].ptrFactor[0] is %d %p\n",i,pstFactors[i].ptrFactor[0],&(pstFactors[i].ptrFactor[0]));
		for(int j = 0;j<=pstFactors[i].ptrFactor[0];j++){
			printf("rank%d:data from pstFactors[%d].ptrFactor[%d] = %d @%p\n",source,i,j,pstFactors[i].ptrFactor[j],&(pstFactors[i].ptrFactor[j]));
		}
	}
	logPstFactors(&pstFactors[0]);
}


void logPstFactors(struct sFactors *pstFactors){
	FILE *fp;
	fp = fopen("logFile.log", "a");
	for(int i = 0; i<pstFactors[0].multiplicity;i++){
		fprintf(fp,"multiplicity from pstFactors[%d] is: %d\n",i,pstFactors[i].multiplicity);
		fprintf(fp,"upperbound from pstFactors[%d] is: %d\n",i,pstFactors[i].upperBound);
		fprintf(fp,"SIZE of ptrFactor[%d] is %d \n",i,pstFactors[i].ptrFactor[0]);
		for(int j = 0;j<=pstFactors[i].ptrFactor[0];j++){
			fprintf(fp,"data from pstFactors[%d].ptrFactor[%d] = %d\n",i,j,pstFactors[i].ptrFactor[j]);
		}
	}
	fclose(fp);

}

void sendPstFactors(struct sFactors *pstFactors,int dest,int source){
	printf("\n\nrank%d: start sending PstFactors to %d\n\n",source,dest);
	int NumberOfElements = pstFactors->multiplicity+1;
//	printf("numberOfMpultiplicities: %d\n",NumberOfElements);

	MPI_Send( &NumberOfElements,1,MPI_INT,dest,NO_OF_PSTFACTORS,MPI_COMM_WORLD);
	for(int i = 0;i<NumberOfElements;i++){
		int numberLength = pstFactors[i].ptrFactor[0]+1;
		MPI_Send(&numberLength,1,MPI_INT,dest,NO_OF_PSTFACTORS,MPI_COMM_WORLD);
		MPI_Send( &(pstFactors[i].multiplicity), 1, MPI_INT, dest, SEND_MULTIPLICITY, MPI_COMM_WORLD);
		MPI_Send( &(pstFactors[i].upperBound), 1, MPI_INT, dest, SEND_UPPERBOUND, MPI_COMM_WORLD);
//		for(int j=0;j<numberLength;j++){
//			printf("value is psti:%d factj:%d  = %d: @%p\n",i,j,pstFactors[i].ptrFactor[j],&(pstFactors[i].ptrFactor[j]));
//		}
//		printf("number of sended elements %d\n",numberLength);
		if( numberLength >= 1 ){
			MPI_Send( &(pstFactors[i].ptrFactor[0]), numberLength , MPI_INT, dest, SEND_PTRFACTOR, MPI_COMM_WORLD);

		}else{
			printf("numberlength is: %d\n",numberLength);
		}

	}
	int len = strlen(tofactorDec);
	MPI_Send(&len,1,MPI_INT,dest,SEND_CHAR,MPI_COMM_WORLD);
	MPI_Send(tofactorDec,len,MPI_CHAR,dest,SEND_CHAR,MPI_COMM_WORLD);
}

void receivePstFactors(struct sFactors *pstFactors,int source,int dest){
	printf("\n\nrank%d: start receiveing PstFactors from %d\n\n",dest,source);
	int numberOfElements;
	MPI_Recv( &numberOfElements,1,MPI_INT,source,NO_OF_PSTFACTORS,MPI_COMM_WORLD,MPI_STATUSES_IGNORE);
//	printf("\n\nnumberOfElements %d\n\n",numberOfElements);
//	int count = 0;
	for(int i = 0;i<numberOfElements;i++){
		MPI_Recv( &(pstFactors[i].multiplicity), 1, MPI_INT, source, SEND_MULTIPLICITY, MPI_COMM_WORLD,MPI_STATUSES_IGNORE);
		//printf("Element: %d WRITE TO %p\n",i,&(pstFactors[i].multiplicity));
		MPI_Recv( &(pstFactors[i].upperBound), 1, MPI_INT, source, SEND_UPPERBOUND, MPI_COMM_WORLD,MPI_STATUSES_IGNORE);
		//printf("Element: %dWRITE TO%p\n",i,&(pstFactors[i].multiplicity));
		MPI_Status status;
		int ptrSize;
		MPI_Recv(&ptrSize,1,MPI_INT,source,NO_OF_PSTFACTORS,MPI_COMM_WORLD,&status);
		//int *temp;
		//printf("number of Elements to receie %d \n",ptrSize);
		pstFactors[i].ptrFactor =  (int *) malloc(ptrSize*sizeof(int));
		int* ptr = pstFactors[i].ptrFactor;
		//printf("waiting for data i: %d  max: %d write to %p\n",i,numberOfElements,ptr);
		if( ptrSize !=0 ){
			MPI_Recv(pstFactors[i].ptrFactor , ptrSize , MPI_INT, source, SEND_PTRFACTOR, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
//			for(int j=0;j<ptrSize;j++){
//				printf("pstFactor[%d].ptrFactor[%d] = %d @%p\n",i,j,pstFactors[i].ptrFactor[j],&(pstFactors[i].ptrFactor[j]));
//			}
		}else{
			pstFactors[i].ptrFactor[0] = 0;
		}

	}
	if(dest == 0){
		writeFactorToDisk(pstFactors);
		printf("factor Saved\n");
	}
	int len;
	MPI_Recv(&len,1,MPI_INT,source,SEND_CHAR,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	MPI_Recv(tofactorDec,len,MPI_CHAR,source,SEND_CHAR,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
}

void showFactors(BigInteger *N,struct sFactors *pstFactors,int world_rank){
	printf("\nRank%d print current factors:\n",world_rank);
	char a[30000];
    char *outString = &a[0];
	char **lt = &outString;
	SendFactorizationToOutput(EXPR_OK,pstFactors,lt,1,a);
	printf("Factorisation is: \n%s\n",a);
}

void saveFactorizationToText(struct sFactors *pstFactors,char *path){
	char a[30000];
    char *outString = &a[0];
	char **lt = &outString;
	SendFactorizationToOutput(EXPR_OK,pstFactors,lt,1,a);
	FILE *fp;
	char buf[1000];
	snprintf(buf, sizeof(buf), "./saves/%s/Factorization.txt",path );
	fp = fopen(buf, "w");
	if(fp != NULL){
		fprintf(fp,"%s\n",a);
		fclose(fp);
		printf("factorization saved to %s\n",buf);
	}else{
		printf("failed to save factorization to txt");
	}
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
void cho_WaitSpecific(MPI_Status *status,int tag){
	int noMess = 0;
	//printf("cho wait\n");
	while(noMess == 0){
		//usleep(100000);
		usleep(500000);
		MPI_Iprobe(MPI_ANY_SOURCE,tag,MPI_COMM_WORLD,&noMess,status);

	}
	//printf("received from %i, tag %i\n",status->MPI_SOURCE,status->MPI_TAG);
}

void saveCurrentEc(int EC){
	FILE *fp;
	char buf[1000];
	snprintf(buf, sizeof(buf), "./saves/%s/currentEC.bin",savePath );
	fp = fopen(buf, "wb");
	if(fp != NULL){
		fwrite(&(EC),sizeof(int),1,fp );
		fclose(fp);
	}else{
		printf("failed to open file:%s\n",buf);
	}
}

void loadCurrentEc(int *EC){
	FILE *fp;
	char buf[1000];
	snprintf(buf, sizeof(buf), "./saves/%s/currentEC.bin",savePath );
	fp = fopen(buf, "rb");
	if(fp != NULL){
		fread(EC,sizeof(int),1,fp );
		fclose(fp);
		printf("currentEC loaded: %d\n",*EC);
	}else{
		printf("failed to open file:%s\n",buf);
	}
}

int cfileexists(const char* filename){
    struct stat buffer;
    int exist = stat(filename,&buffer);
    if(exist == 0)
        return 1;
    else // -1
        return 0;
}

void writeFactorToDisk(struct sFactors *pstFactors){
	int NumberOfElements = pstFactors->multiplicity+1;
	char buf[1000];
	snprintf(buf, sizeof(buf), "saves/%s",savePath );
	if(cfileexists(buf)==0){
		mkdir(buf,0777);
//		umask()
	}else{
		printf("folder exists\n");
	}

	printf("write to disk\n%s",buf);
	for(int i = 0;i<NumberOfElements;i++){
		snprintf(buf, sizeof(buf), "./saves/%s/sFactor%i.bin",savePath,i );
		FILE *fp;
		fp = fopen( buf, "wb");
		if(fp != NULL){
			int numberLength = pstFactors[i].ptrFactor[0]+1;
			fwrite(&(numberLength),sizeof(int),1,fp );
	//		printf("write numberlength %d\n",numberLength);
			fwrite(&(pstFactors[i].multiplicity),sizeof(int),1,fp );
	//		printf("write multipli %d\n",pstFactors[i].multiplicity);
			fwrite(&(pstFactors[i].upperBound),sizeof(int),1,fp );
	//		printf("write upper %d\n",pstFactors[i].upperBound);
			fwrite(&(pstFactors[i].ptrFactor[0]),sizeof(int),numberLength,fp);
	//		printf("ptr is: ");
			for(int j=0;j<numberLength;j++){
				printf("%d ",pstFactors[i].ptrFactor[j]);
			}
			printf("\n");
			fclose(fp);
		}else{
			printf("failed to open file:%s\n",buf);
		}

	}
	saveFactorizationToText(pstFactors,savePath);
}
void readFactorFromDisk(struct sFactors *pstFactors,char* pathToFolder){
	char buf[1000];
	int fileNr = 0;
	printf("path to folder: %s\n",pathToFolder);
	int* currentPosition;
	currentPosition = pstFactors->ptrFactor;
	for(;;){
		snprintf(buf, sizeof(buf), "./saves/%s/sFactor%i.bin",pathToFolder,fileNr );
		printf("read from %s\n",buf);

		FILE *fp;
		fp = fopen( buf, "rb");
		if(fp != NULL){
			int ptrValueLength = 0;
			printf("load factor %d\n",fileNr);
			fseek(fp, 0, SEEK_SET);
			fread(&(ptrValueLength),sizeof(int),1,fp );
			printf("read ptrValue length %d\n",ptrValueLength);
			fread(&(pstFactors[fileNr].multiplicity),sizeof(int),1,fp );
			printf("read multi %d\n",pstFactors[fileNr].multiplicity);
			fread(&(pstFactors[fileNr].upperBound),sizeof(int),1,fp );
			printf("read upper %d\n",pstFactors[fileNr].upperBound);
			pstFactors[fileNr].ptrFactor = currentPosition;
			fread(currentPosition,sizeof(int),ptrValueLength,fp);
			currentPosition+=ptrValueLength;
			fclose(fp);
			printf("factor %d loaded\n",fileNr);
			fileNr++;
			if(fileNr>pstFactors[0].multiplicity){
				if(pstFactors[fileNr-1].upperBound == 0){
					pstFactors[fileNr-1].upperBound = 2;
				}

				break;
			}
		}else{
			printf("Could not open file: %s\n",buf );
			break;
		}
	}
	printf("%d Factors loaded",pstFactors[0].multiplicity);
}


void setSavePoint(char *tofactor){
	strcpy(savePath,tofactor);
	printf("foldername set to %s\n",tofactor);
}
