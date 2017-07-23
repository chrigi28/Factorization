#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
void cho_Waitany(MPI_Status *status);

int main(int argc, char** argv) {
    // Initialize the MPI environment
	printf("hallo MPI world\n");
    MPI_Init(NULL, NULL);
    int outBuffer[10];
    MPI_Status status;
    // Get the number of processes
    int world_size;
    int world_rank; // my rank
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    //get own rank
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    if (world_rank == 0){ //taskhandler
//    	MPI_Request request[2*world_size];
//    	for(int i=0;i<world_size;i++){
//    		request[i]=MPI_REQUEST_NULL;
//    	}
    	for(int i = 0;i<world_size-1;i++){
			cho_Waitany(&status);
			int tag = status.MPI_TAG;
			int rank = status.MPI_SOURCE;
			int count = 0;
			MPI_Get_count(&status,MPI_INT,&count);
			MPI_Recv(&count,count,MPI_INT,rank,tag ,MPI_COMM_WORLD,&status);

			printf("Message reached: %d\n",count);
    	}
    }else{
    	sleep(30);
    	int data = world_rank*1435;
    	MPI_Send(&data,1,MPI_INT,0,43,MPI_COMM_WORLD);

    }

    printf("process finished finalize mpiworld: %i\n",world_rank);
    // Finalize the MPI environment.
    MPI_Finalize();
}


void cho_Waitany(MPI_Status *status){
	int noMess = 0;
	printf("cho wait\n");
	while(noMess == 0){
		//usleep(100000);

		usleep(500000);
		MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&noMess,status);

	}
	//printf("received from %i, tag %i\n",status->MPI_SOURCE,status->MPI_TAG);
}
