#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "bignbr.h"
#include "highlevel.h"
#include "factor.h"
#include "helper.h"
#include <limits.h>


#define STARTPARALLEL 9
const int TERMINATE_MESSAGE = -1;

int main(int argc, char** argv) {
	// Initialize the MPI environment
	time_t start = time(NULL);
	time_t end;
	int i = 0;
	int factoringRunning = 0;
	int factoringDone = 0;
	MPI_Init(NULL, NULL);


	BigInteger N;
	struct sFactors pstFactors[1000];
	MPI_Status status;
	// Get the number of processes
	int world_size;
	int world_rank; // my rank
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	//get own rank
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	if (world_rank == 0) { //taskhandler
		setSavePoint(argv[1]);
		int index = 0;
		int currentEC = 1; //1 starts automaticly by rank 1
		int currentAnswerPosition = 0;
		struct {
			int list[world_size];
			int idx;
		}canceledEC;
		memset(canceledEC.list,0,sizeof(int)*world_size);
		canceledEC.idx=0;
//	    BigInteger N;
//	    struct sFactors pstFactors;
//
//		MPI_Request request[world_size];
//		for (int i = 0; i < world_size; i++) {
//			request[i] = MPI_REQUEST_NULL;
//		}

//		char inBuffer[world_size][ANSWER_SIZE];
//		int receiveBuffer[world_size];
//		memset(receiveBuffer, 0, sizeof(receiveBuffer[0]) * world_size);

		int pendingAnswers = world_size - 1;

		struct waitList {
			int idx;
			int list[world_size];
		} rdyToFactor;
		int runningEClist[world_size];
		memset(runningEClist, -1, sizeof(int) * world_size);
		memset(&(rdyToFactor.list[0]), -1, sizeof(rdyToFactor.list[0]) * world_size);
		rdyToFactor.idx = 0;

		while (pendingAnswers > 0) {
			//printf("main0: pending answers %i\n", pendingAnswers);
			//printf("wait for any comm\n");
//			int waitLength = world_size;
			printf("\nrank0: wait for next Request: \n");
			int rank = -1;
			cho_Waitany(&status);
			rank = status.MPI_SOURCE;
			int tag = status.MPI_TAG;
			int count = -1;
//    		MPI_Waitany(pendingAnswers+1,request,&rank,&status);
			MPI_Get_count(&status, MPI_INT, &count);

			char *t1 = STATENAMES[tag];
			printf("\nRequest received Tag%d: %s from %d\n", status.MPI_TAG, t1,rank);
			MPI_Recv(&count, count, MPI_INT, rank, tag, MPI_COMM_WORLD,&status);

			switch (tag) {
			case REGISTER_FOR_EC: {
//				printf("main0: register_for ec received from %d\n", rank);

				if (factoringRunning == 1 && currentEC>STARTPARALLEL) {
					MPI_Send(&factoringRunning, 1, MPI_INT, rank,REGISTER_FOR_EC, MPI_COMM_WORLD);
					printf("send Register_for_EC to rank%d\n",rank);
//					printf("main0: Send FactorData to rank:%d\n", rank);
//					sendBigInteger(&N, rank,world_rank);
////					printf("main0:printing before sending\n ");
////					printPstFactors(&pstFactors[0],world_rank);
//					sendPstFactors(&pstFactors[0], rank,world_rank);
//					// MPI_Irecv(&receiveBuffer[rank], 1, MPI_INT, rank ,MPI_ANY_TAG, MPI_COMM_WORLD, &request[rank]);
				}else{
					if(factoringDone){
						int cancel = -1;
						MPI_Send(&cancel, 1, MPI_INT, rank,REGISTER_FOR_EC, MPI_COMM_WORLD);
						printf("send Register_for_EC to rank%d with cancel\n",i);
					}else{
						rdyToFactor.list[rank] = 1;
						printf("main0: rank %d added to waitlist\n",rank);
					}
				}
//					printf("main0: register_for ec received %d\n",rank);
			}
				break;
			case SEND_RDY_FACTOR: {
				printf("main0: SEND_RDY_FACTOR received from rank %d\n", rank);
				//MPI_Recv(&receiveBuffer[rank],1,MPI_INT,rank,SEND_RDY_FACTOR,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
//    				printf("main0: rdy fac rec  send job\n");
				//argv[1]
//    				MPI_Send(&argv[1],(int)strlen(argv[1]),MPI_CHAR,rank,JOBTOFACTOR,MPI_COMM_WORLD);
//    				printf("main0: job sended \n");
//    				MPI_Irecv(&null, 1, MPI_INT, rank ,START_FACTORING, MPI_COMM_WORLD, &request[rank]);
			}
				break;
			case START_FACTORING:
				;
				printf("\nmain0: startFactoring\n");
				//MPI_Recv(&null, 1, MPI_INT, rank,START_FACTORING, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

				receiveBigInteger(&N, rank,world_rank);
				printf("\nmain0: bigint received\n");
				receivePstFactors(&pstFactors[0], rank,world_rank);
				printf("\nafter received pstfactors\n");
				showFactors(&N,pstFactors,world_rank);
				printf("MAIN0: printing pstFactors after receiving\n");
				//printPstFactors(&pstFactors[0],world_rank);
				printf("\nmain0: bigint and pstfactor received\n\n");
				factoringRunning = 1;
//    			    MPI_Send(&currentEC,1,MPI_INT,rank,START_FACTORING,MPI_COMM_WORLD);
//				if(currentEC>20){
//					for (int i = 1; i < world_size; i++) {
//	//    			    	printf("main0: \nforloop idx: %d\n",i);
//						if(rdyToFactor.list[i]==1){
//							MPI_Send(&i, 1, MPI_INT, i, REGISTER_FOR_EC,MPI_COMM_WORLD);
//							rdyToFactor.list[i]=0;
//						}
//						//currentEC++;
//	//					sendBigInteger(&N, i,world_rank);
//	////    			        printf("main0: bigint sent\n");
//	//					sendPstFactors(pstFactors, i,world_rank);
//	//					printf("main0: send FactorData to rank:%d\n",i);
//					}
//				}
					//    			    MPI_Irecv(&receiveBuffer[rank], 1, MPI_INT, rank,MPI_ANY_TAG, MPI_COMM_WORLD, &request[rank]);
				break;

			case SEND_EC:
				//MPI_Recv(&null,1,MPI_INT,rank,SEND_EC,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
//				printf("main0: ec request received from %d\n", rank);
				;

				int answ;
				if(factoringRunning){
					if(canceledEC.idx==0){
						saveCurrentEc(currentEC-1);
						answ = currentEC;
					}else{
						int lowestProcessedEC=INT_MAX;
						for(int i=0;i<canceledEC.idx;i++){
							int currentNumber = canceledEC.list[i];
							if(currentNumber != -1 && currentNumber < lowestProcessedEC){
								lowestProcessedEC = currentNumber-1;
							}
						}
						saveCurrentEc(lowestProcessedEC);
						answ = canceledEC.list[canceledEC.idx];
						canceledEC.idx--;
					}
					rdyToFactor.list[rank] = 0;
					if(currentEC>STARTPARALLEL){
						for(int i=2;i<world_size;i++){
							if(rdyToFactor.list[i]==1){
								MPI_Send(&i, 1, MPI_INT, i, REGISTER_FOR_EC,MPI_COMM_WORLD);
								printf("send Register_for_EC to rank%d\n",i);
								rdyToFactor.list[i]=0;
							}
						}
					}

					runningEClist[rank] = currentEC;

					currentEC++;

				}else{
					answ = -1;
				}
				MPI_Send(&answ, 1, MPI_INT, rank, SEND_EC, MPI_COMM_WORLD);


				//    				MPI_Irecv(&receiveBuffer[rank], 1, MPI_INT, rank,MPI_ANY_TAG, MPI_COMM_WORLD, &request[rank]);
				printf("main0: nextEC is : %d\n", currentEC);
				break;

			case CHECK_FACTOR:
				printf("\n\n ==========================factor found========================\n\n");
				printf("main0: Rank %d found, check factor\n",rank);
				rdyToFactor.list[rank] = 1;
				runningEClist[rank] = -1;
				limb GD[MAX_LEN];
				for(int i=1;i<world_size;i++){
					if(runningEClist[i] > -1){
						//cancel all running processes
//							printf("send Cancel to rank%d\n",i);
						int cancel=-1;
						MPI_Send(&cancel,1,MPI_INT,i,INTERRUPT_EC,MPI_COMM_WORLD);
						canceledEC.list[canceledEC.idx] = runningEClist[i];
						runningEClist[i] = -1;
						canceledEC.idx++;
					}
				}
				int NumberLength;
//				printf("send rdy to get numberlength\n");
				MPI_Send(&NumberLength, 1, MPI_INT, rank, CHECK_FACTOR, MPI_COMM_WORLD);
//				printf("receiv numberlength\n");
				MPI_Recv(&NumberLength, 1, MPI_INT, rank, CHECK_FACTOR,	MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//				printf("send rdy to get GD\n");
				MPI_Send(&NumberLength, 1, MPI_INT, rank, CHECK_FACTOR, MPI_COMM_WORLD);
				memset(&GD[1], 0, (NumberLength - 1) * sizeof(limb));
//				printf("main0: numberlenght received from Rank %d  get GD\n",rank);
				MPI_Recv(&GD[0], MAX_LEN, MPI_INT, rank,CHECK_FACTOR, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				char newFactor[200];
				Bin2Dec(GD, newFactor, NumberLength,6);
				printf("newFactor is %s\n",newFactor);
				int restart=1;
//				if(rank != 1){
//					printf("interupt rank1\n");
//					MPI_Send(&restart,1,MPI_INT,1,INTERRUPT_EC,MPI_COMM_WORLD);
//					canceledEC.list[canceledEC.idx] = runningEClist[1];
//					canceledEC.idx++;
//					runningEClist[1]=-1;
//				}

				MPI_Send(&restart,1,MPI_INT,1,CHECK_FACTOR,MPI_COMM_WORLD); //indicate communication
//				printf("send numberlength\n");
				MPI_Send(&NumberLength,1,MPI_INT,1,CHECK_FACTOR,MPI_COMM_WORLD);
//				printf("send gd\n");
				MPI_Send(&GD[0],MAX_LEN,MPI_INT,1,CHECK_FACTOR,MPI_COMM_WORLD);
				int factor;
//				printf("wait for answer\n");
				MPI_Recv(&factor, MAX_LEN, MPI_INT, 1,CHECK_FACTOR, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				if(factor){
					factoringRunning = 0;
					receivePstFactors(pstFactors,1,0);
//					printf("\n\n\nMAIN0: factoring after check \n\n\n\n");
					showFactors(&N,pstFactors,world_rank);


				}
				factoringRunning=1;
				break;
			case GET_FACTOR_DATA:
				sendBigInteger(&N,rank,0);
				sendPstFactors(pstFactors,rank,0);
				break;
			case FACTORING_DONE:
				end = time(NULL);
				printf("\n\n time Needed to factorize %.2f seconds\n", (double)(end- start));
				factoringRunning = 0;
				factoringDone = 1;
				int cancel =-1;
				for(int i=0;i<world_size;i++){
					if(rdyToFactor.list[i]==1){
						MPI_Send(&cancel, 1, MPI_INT, i, REGISTER_FOR_EC,MPI_COMM_WORLD);
						printf("send Register_for_EC to rank%d cancel\n",i);
					}
					if(runningEClist[i]!=-1){
						MPI_Send(&restart,1,MPI_INT,1,INTERRUPT_EC,MPI_COMM_WORLD);
						runningEClist[i]=-1;
					}
				}
				break;
			case LOAD_EC:
				loadCurrentEc(&currentEC);
				printf("EC loaded set to %d\n",currentEC);
				break;
			case LOGOUT:
				pendingAnswers--;
				rdyToFactor.list[rank]=-1;
				runningEClist[rank] = -1;
				break;
			default:
				printf("\n\n ==========================ERROR========================\n\n");
				printf("undefined TAG was: %d", status.MPI_TAG);
				return -1;
				break;
			}

		}    				// end while pending
		printf("end master\n");
		MPI_Finalize();
		return 0;
	}    				//end taskhandler
	else { //worker
		printf("rank %d: worker\n", world_rank);

		if (world_rank == 1) {
			printf("rank1: send rdy factor %s\n",argv[2]);
//			printf("No of arguments: %d\n",argc);
			if((argc > 2)&&(strcmp(argv[2],"-continue")==0)){
				printf("continue factorization: %s\n",argv[2]);
				ecmFrontText(argv[1], 1, argv[1], 1);
			}else{
				printf("start new factorization\n");
				ecmFrontText(argv[1], 1, NULL, 1);
			}


		} else {
			int EC;
			while (1) {
				int found = 0;
//				printf("Rank%d: register for for EC\n", world_rank);
				MPI_Send(&world_rank, 1, MPI_INT, 0, REGISTER_FOR_EC,MPI_COMM_WORLD);
				cho_Waitany (&status);
//    			printf("rank%d: mes received tag = %d \n",world_rank,status.MPI_TAG);
				if(status.MPI_TAG == REGISTER_FOR_EC){
					MPI_Recv(&EC, 1, MPI_INT, 0, REGISTER_FOR_EC, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				}else{
					printf("rank%d: received tag: %d",world_rank,status.MPI_TAG);
					MPI_Recv(&EC, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				}
//				printf("Rank%d: EC received %d\n", world_rank, EC);
				if (EC != -1) {
//						receiveBigInteger(&N, 0,world_rank);
//						printf("Rank%d: bigint received\n", world_rank);
//						receivePstFactors(&pstFactors[0], 0,world_rank);
//	//					printPstFactors(&pstFactors[0],world_rank);
//						showFactors(&N,pstFactors,world_rank);
					ecmParallel(&N, &pstFactors[0], world_rank);
					printf("rank%d: returned from ecmParallel\n", world_rank);
				} else {
					printf("rank%d: Closes EC received :%d\n", world_rank, EC);
					break;
				}
//				}
//				else{
//					printf("Rank%d: received mess tag:%d from %d",world_rank,status.MPI_TAG,status.MPI_SOURCE);
//					int count;
//					int test[1000];
//					MPI_Get_count(&status,MPI_INT,&count);
//					MPI_Recv(&test, count, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//				}
			} //while

		} //else of if(1)

	} //end worker

	printf("process finished finalize mpiworld: %i\n", world_rank);
	// Finalize the MPI environment.
	MPI_Send(&world_rank, 1, MPI_INT, 0, LOGOUT,MPI_COMM_WORLD);
	MPI_Finalize();
	for(int i=0;i<pstFactors[0].multiplicity;i++){
		free(pstFactors[i].ptrFactor);
	}
	return 0;
}

