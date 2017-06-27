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
void cho_Waitany(MPI_Status *status);



#include "helper.h"
int null = 0;
const int ANSWER_SIZE = 2000;
const int TERMINATE_MESSAGE = -1;

int main(int argc, char** argv) {
    // Initialize the MPI environment
	 int i = 0;
	int factoringRunning = 0;
    MPI_Init(NULL, NULL);
    enum Messages message = GET_JOB;
    int outBuffer[10];
	BigInteger N;
	struct sFactors pstFactors;
    MPI_Status status;
    // Get the number of processes
    int world_size;
    int world_rank; // my rank
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    //get own rank
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    if (world_rank == 0){ //taskhandler
		int index = 0;
		int currentEC = 1; //1 starts automaticly by rank 1
		int currentAnswerPosition=0;
//	    BigInteger N;
//	    struct sFactors pstFactors;

    	MPI_Request request[world_size];
    	for(int i=0;i<world_size;i++){
    		request[i]=MPI_REQUEST_NULL;
    	}
    	//char answers[numberOfTotalTasks][ANSWER_SIZE];
    	//memset(answers,0,sizeof(answers[0][0])*numberOfTotalTasks*ANSWER_SIZE);
    	char inBuffer[world_size][ANSWER_SIZE];
    	int receiveBuffer[world_size];
    	memset(receiveBuffer,0,sizeof(receiveBuffer[0])*world_size);
//    	for(int i=0;i<world_size;i++){
//    		char answer[ANSWER_SIZE];
//    		memset(answer,0,sizeof(answer[0])*ANSWER_SIZE);
//    		inBuffer[i] = answer;
//    	}
    	int null;
    	int pendingAnswers=0;
    	for(int i=1;i<world_size;i++){
			printf("all register comm i: %d\n",i);
    		if(i == 1){
				message = SEND_RDY_FACTOR;
			}else{
				message = REGISTER_FOR_EC;
			}
    		MPI_Irecv(&null, 1, MPI_INT, i,message, MPI_COMM_WORLD, &request[i]);
			pendingAnswers++;
    	}
    	pendingAnswers=world_size-1;

    	struct waitList{
    		int idx;
    		int list[world_size];
    	}waitList;
    	memset(&(waitList.list[0]),-1,sizeof(waitList.list[0])*world_size);
    	waitList.idx = 0;
    	while(pendingAnswers > 0 ){
    		printf("main0: pending answers %i\n",pendingAnswers);
    		//printf("wait for any comm\n");
    		int waitLength = world_size;
    		//cho_Waitany(&status);
    		int rank = -1;
    		MPI_Waitany(pendingAnswers+1,request,&rank,&status);
    		//int rank = status.MPI_SOURCE;
    		printf("\nTag: %d\n",status.MPI_TAG);
    		switch(status.MPI_TAG){
    			case REGISTER_FOR_EC:{
    				printf("main0: register_for ec received %d\n",rank);
//    				int rcount = 0;
//					MPI_Get_count(&status,MPI_INT,&rcount);
//					int receiveBuffer[rcount];
//					MPI_Recv(&receiveBuffer,rcount,MPI_INT,rank,REGISTER_FOR_EC,MPI_COMM_WORLD,&status);
					waitList.list[waitList.idx] = rank;
					waitList.idx++;
					if(factoringRunning == 1){
						MPI_Send(&factoringRunning,1,MPI_INT,0,REGISTER_FOR_EC,MPI_COMM_WORLD);
    			        sendBigInteger(&N,rank);
    			        sendPstFactors(&pstFactors,rank);
    			        MPI_Irecv(&receiveBuffer[rank], 1, MPI_INT, rank ,MPI_ANY_TAG, MPI_COMM_WORLD, &request[rank]);
					}
//					printf("main0: register_for ec received %d\n",rank);
    			}
				break;
    			case SEND_RDY_FACTOR:{
//    				printf("main0: SEND_RDY_FACTOR\n");
    				//MPI_Recv(&receiveBuffer[rank],1,MPI_INT,rank,SEND_RDY_FACTOR,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
//    				printf("main0: rdy fac rec  send job\n");
    				//argv[1]
//    				printf("main0: length is : %d\n",strlen(argv[1]));
    				MPI_Send(argv[1],strlen(argv[1]),MPI_CHAR,rank,JOBTOFACTOR,MPI_COMM_WORLD);
//    				printf("main0: job sended \n");
    				MPI_Irecv(&null, 1, MPI_INT, rank ,START_FACTORING, MPI_COMM_WORLD, &request[rank]);
    			}
    			break;
    			case START_FACTORING: ;
    				printf("\nmain0: startFactoring\n");
    				//MPI_Recv(&null, 1, MPI_INT, rank,START_FACTORING, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

    				receiveBigInteger(&N,rank);
    				printf("\nmain0: bigint received\n");
    			    receivePstFactors(&pstFactors,rank);

    			    printf("main0: bigint and pstfactor received\n");
    			    factoringRunning = 1;
    			    int i;
    			    for(i = 0;i<waitList.idx;i++){
//    			    	printf("main0: \nforloop idx: %d\n",i);
    			    	MPI_Send(&i,1,MPI_INT,waitList.list[i],REGISTER_FOR_EC,MPI_COMM_WORLD);
    			        //currentEC++;
    			        sendBigInteger(&N,waitList.list[i]);
//    			        printf("main0: bigint sent\n");
    			        sendPstFactors(&pstFactors,waitList.list[i]);

    			        MPI_Irecv(&receiveBuffer[waitList.list[i]], 1, MPI_INT, waitList.list[i],MPI_ANY_TAG, MPI_COMM_WORLD, &request[waitList.list[i]]);
    			    }
    			    printf("RANK TO RECEIVE FROM %d \n",rank);
    			    MPI_Irecv(&receiveBuffer[rank], 1, MPI_INT, rank,MPI_ANY_TAG, MPI_COMM_WORLD, &request[rank]);
    			break;

    			case SEND_EC:
    				//MPI_Recv(&null,1,MPI_INT,rank,SEND_EC,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    				printf("main0: ec request received from %d\n",rank);
    				MPI_Send(&currentEC,1,MPI_INT,rank,SEND_EC,MPI_COMM_WORLD);
    				currentEC++;
    				MPI_Irecv(&receiveBuffer[rank], 1, MPI_INT, rank,MPI_ANY_TAG, MPI_COMM_WORLD, &request[rank]);
    				printf("main0: nextEC is : %d\n",currentEC);
    			break;

    			default:
    				printf("undefined TAG was: %d",status.MPI_TAG);
    				return -1;
    			break;
    		}

    	}// end while pending
    	printf("end master\n");
        MPI_Finalize();
        return 0;
    }//end taskhandler
    else{ //worker
    	printf("rank %d: worker\n",world_rank);

    	int incoming_msg_size;
    	if(world_rank == 1){
    		printf("rank1: send rdy factor\n");
    		MPI_Send(&null,1,MPI_INT,0,SEND_RDY_FACTOR,MPI_COMM_WORLD);
//    		printf("rank1: wait for jobtofactor\n");
    		MPI_Probe(0,JOBTOFACTOR,MPI_COMM_WORLD,&status);
//    		printf("3\n");
    	    MPI_Get_count(&status, MPI_CHAR, &incoming_msg_size);
//    	    printf("4 count: %d\n",incoming_msg_size);
    	    char receive[incoming_msg_size];
    	    MPI_Recv(&receive,incoming_msg_size,MPI_CHAR,0,JOBTOFACTOR,MPI_COMM_WORLD,&status); //term to factor as char array (as commandline)
//    		printf("startFrontTExt\n");
    	    ecmFrontText(argv[1], 1, NULL);
    	}
    	else{
    		printf("Rank%d: sendRdy for ec:()\n",world_rank);
//    		sleep(3);
    		int a = 1;
    		MPI_Send(&a,1,MPI_INT,0,REGISTER_FOR_EC,MPI_COMM_WORLD);

    		int EC;
    		printf("Rank%d: waiting for EC\n",world_rank);
    	    MPI_Recv(&EC,1,MPI_INT,0,REGISTER_FOR_EC,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    	    printf("Rank%d: EC received\n",world_rank);
    		if(EC != -1){

    			receiveBigInteger(&N,0);
    			printf("Rank%d: bigint received\n",world_rank);
    			receivePstFactors(&pstFactors,0);
    			//irecv for cancel or update
    			printf("Rank%d: data received start ecmParallel\n",world_rank);
    			ecmParallel(&N, &pstFactors,world_rank);
    		}


    	}

    }//end worker

    printf("process finished finalize mpiworld: %i\n",world_rank);
    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
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
