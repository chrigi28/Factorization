#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

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
    	int startParameter = (int)*argv[1];
		int endParameter = (int)*argv[2];
		int numberOfTotalTasks= endParameter-startParameter;
		int tasksLeft = numberOfTotalTasks;
		int receivedRank;
		int currentParameter = startParameter;
		int currentAnswerPosition=0;

    	MPI_Request request[2*world_size];
    	for(int i=0;i<2*world_size;i++){
    		request[i]=MPI_REQUEST_NULL;
    	}
    	char* answers[numberOfTotalTasks];
    	void* inBuffer[world_size];

    	for(int i=0;i<world_size;i++){
    		char answer[ANSWER_SIZE];
    		inBuffer[i] = answer;
    	}
    	int null;
    	int pendingAnswers=0;
    	for(int i=1;i<world_size;i++){
			MPI_Irecv(&null, 1, MPI_INT, i,message, MPI_COMM_WORLD, &request[i]);
			pendingAnswers++;
    	}
    	while(pendingAnswers > 0 ){
    		printf("wait for any comm\n");
    		MPI_Waitany(world_size*2, &request[1], &receivedRank, &status);
    		pendingAnswers--;
    		if(receivedRank < world_size){ // wenn jobrequest
        		message = SEND_JOB;
        		MPI_Send(&currentParameter,1,MPI_INT,receivedRank,message,MPI_COMM_WORLD);
        		currentParameter++;
        		tasksLeft--;
        		MPI_Irecv(&inBuffer[receivedRank], ANSWER_SIZE, MPI_CHAR, receivedRank, SEND_RESULT, MPI_COMM_WORLD, &request[world_size+receivedRank]);
        		pendingAnswers++;

    		}else{ //answer received
    			answers[currentAnswerPosition] = inBuffer[receivedRank];
    			currentAnswerPosition++;
    			if(tasksLeft > 0){
            		message = SEND_JOB;
            		MPI_Send(&currentParameter,1,MPI_INT,receivedRank,message,MPI_COMM_WORLD);
            		currentParameter++;
            		tasksLeft--;
            		MPI_Irecv(&inBuffer[receivedRank], ANSWER_SIZE, MPI_CHAR, receivedRank, SEND_RESULT, MPI_COMM_WORLD, &request[world_size+receivedRank]);
            		pendingAnswers++;
				}else{ //no job left send finish job
					MPI_Send(&TERMINATE_MESSAGE,1,MPI_INT,receivedRank,message,MPI_COMM_WORLD);
				}
    		}//END ANSWER RECEIVED
    	}// end while pending

    }//end taskhandler
    else{ //worker
    	int parameter;
    	MPI_Status status;
    	int dummy = 1;
    	MPI_Send(&dummy,1,MPI_INT,0,GET_JOB,MPI_COMM_WORLD);
    	MPI_Recv(&parameter, 1, MPI_INT, 0, GET_JOB, MPI_COMM_WORLD,&status);
    	while(parameter !=-1){
			printf("doCalculation for %i",parameter);
			sleep(2);
			char answer[ANSWER_SIZE];
			snprintf(answer,sizeof(answer),"Result from Rank %i",world_rank);
			MPI_Send(&answer,ANSWER_SIZE,MPI_CHAR,0,SEND_RESULT,MPI_COMM_WORLD);
			MPI_Recv(&parameter, 1, MPI_INT, 0, GET_JOB, MPI_COMM_WORLD,&status);
    	}
    }
    // Finalize the MPI environment.
    MPI_Finalize();
}

