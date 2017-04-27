#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//test copy
#include "bignbr.h"
#include "highlevel.h"
#include "factor.h"
void ecmFrontText(char *tofactorText, int doFactorization, char *knownFactors);
//end copy




enum Messages {GET_JOB,SEND_JOB,SEND_RESULT};
const int ANSWER_SIZE = 2000;
const int TERMINATE_MESSAGE = -1;

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    enum Messages message = GET_JOB;
    int outBuffer[10];

    MPI_Status status;
    // Get the number of processes
    int world_size;
    int world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    //get own rank
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    if (world_rank == 0){ //taskhandler
    	int startParameter = atoi(argv[1]);
		int endParameter = atoi(argv[2]);
		printf("parameters are %i to %i\n",startParameter,endParameter);
		int numberOfTotalTasks= endParameter-startParameter;
		int tasksLeft = numberOfTotalTasks;
		int index;
		int currentParameter = startParameter;
		int currentAnswerPosition=0;

    	MPI_Request request[2*world_size];
    	for(int i=0;i<2*world_size;i++){
    		request[i]=MPI_REQUEST_NULL;
    	}
    	char answers[numberOfTotalTasks][ANSWER_SIZE];
    	memset(answers,0,sizeof(answers[0][0])*numberOfTotalTasks*ANSWER_SIZE);
    	char inBuffer[world_size][ANSWER_SIZE];

//    	for(int i=0;i<world_size;i++){
//    		char answer[ANSWER_SIZE];
//    		memset(answer,0,sizeof(answer[0])*ANSWER_SIZE);
//    		inBuffer[i] = answer;
//    	}
    	int null;
    	int pendingAnswers=0;
    	for(int i=1;i<world_size;i++){
			MPI_Irecv(&null, 1, MPI_INT, i,message, MPI_COMM_WORLD, &request[i]);
			pendingAnswers++;
    	}
    	while(pendingAnswers > 0 ){
    		//printf("wait for any comm\n");
    		int waitLength = world_size*2;
    		MPI_Waitany(waitLength, &request[0], &index, &status);
    		pendingAnswers--;
    		//MPI_Comm_size(MPI_COMM_WORLD, &world_size); //world_size gets changed but before!!
    		//printf("\nwait done index is %i world is %i\n",index,world_size);
    		if(index < world_size){ // wenn jobrequest
    			//printf("index is: %i, < world_size is: %i\n",index,world_size);
        		//message = SEND_JOB;
        		//MPI_Send(&currentParameter,1,MPI_INT,index,SEND_JOB,MPI_COMM_WORLD);
        		//printf("zeile 60 send to Idx: %i\n",index);
    			MPI_Send(&currentParameter,1,MPI_INT,index,SEND_JOB,MPI_COMM_WORLD);
        		//printf("currentParameter: %i\n",currentParameter);
        		currentParameter++;
        		//printf("currentParameter: %i\n",currentParameter);
        		tasksLeft--;

//        		MPI_Irecv(&inBuffer[index], ANSWER_SIZE, MPI_CHAR, index, SEND_RESULT, MPI_COMM_WORLD, &request[world_size+index]);
        		//get results
        		MPI_Irecv(&inBuffer[index][0], ANSWER_SIZE, MPI_CHAR, index, SEND_RESULT, MPI_COMM_WORLD, &request[world_size+index]);
        		pendingAnswers++;

    		}else{ //answer received
    			//printf("Result received from rank: %i\n",index-world_size);
    			//strcpy(answers[currentAnswerPosition],inBuffer[index-world_size]);
    			strcpy(answers[currentAnswerPosition],inBuffer[index-world_size]);
    			currentAnswerPosition++;
    			if(tasksLeft > 0){
    				MPI_Send(&currentParameter,1,MPI_INT,1,SEND_JOB,MPI_COMM_WORLD);
    				currentParameter++;
            		tasksLeft--;
            		MPI_Irecv(&inBuffer[index-world_size][0], ANSWER_SIZE, MPI_CHAR, 1, SEND_RESULT, MPI_COMM_WORLD, &request[index]);
            		pendingAnswers++;
				}else{ //no job left send finish job
					//printf("send terminate mes to: %i\n",index-world_size);
					MPI_Send(&TERMINATE_MESSAGE,1,MPI_INT,index-world_size,SEND_JOB,MPI_COMM_WORLD);
				}
    		}//END ANSWER RECEIVED
    	}// end while pending
    	for(int i=0;i<numberOfTotalTasks;i++){
    		printf("%s\n",&answers[i][0]);
    	}
    }//end taskhandler
    else{ //worker
    	printf("worker: %i started\n",world_rank);
    	int parameter;
    	MPI_Status status;
    	int dummy = 1;
    	//request job
    	MPI_Send(&dummy,1,MPI_INT,0,GET_JOB,MPI_COMM_WORLD);
    	//wait for job
    	MPI_Recv(&parameter, 1, MPI_INT, 0, SEND_JOB, MPI_COMM_WORLD,&status);
    	//printf("worker received job P=%i\n",parameter);
    	while(parameter !=TERMINATE_MESSAGE){
			//printf("doCalculation for %i\n",parameter);
			//sleep(1);
    		char toFactorChar[20000];
    		char answer[20000];
    	    //sprintf(toFactorChar,"%i",parameter);
    	    //strcpy(toFactorChar,"99999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999\0");
    	    //int len = strlen(toFactorChar);
    	    //printf("factorlen: %i\n",len);
    		sprintf(toFactorChar,"10^%i+1",parameter);
    	    ecmFrontText(toFactorChar, 1, NULL);
    	    sprintf(answer,"%s\n", output);
			//char answer[ANSWER_SIZE];
			//snprintf(answer,sizeof(answer),"my result is Rank %i\n",world_rank);
			//send the result
			MPI_Send(answer,ANSWER_SIZE,MPI_CHAR,0,SEND_RESULT,MPI_COMM_WORLD);
			//get next job
			MPI_Recv(&parameter, 1, MPI_INT, 0, SEND_JOB, MPI_COMM_WORLD,&status);
    	}//end while
    }//end worker
    //printf("process finished finalize mpiworld: %i\n",world_rank);
    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}

