
#include "mpi.h"

/*
 *
 * MPI_Send(
    void* data,
    int count,
    MPI_Datatype datatype,
    int destination,
    int tag,
    MPI_Comm communicator)
 *
 *
 * MPI_Recv(
    void* data,
    int count,
    MPI_Datatype datatype,
    int source,
    int tag,
    MPI_Comm communicator,
    MPI_Status* status)
 *
 *
 * int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request *request)
 *
 * int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
              MPI_Comm comm, MPI_Request *request)

 *
 *	int MPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status);
 *
 *
 *
 *
 *
 */

enum Messages {
	GET_JOB,
	SEND_JOB,
	SEND_RESULT,
	MPI_GetNextEC,
	MPI_SendNextEC,
	MPI_FactorFound
	};

int getNextECToCompute(int myRank){
	int EC = 0;
	MPI_Send(&EC,1,MPI_INT,myRank,MPI_GetNextEC,MPI_COMM_WORLD); //ask for new EC
	MPI_Recv(&EC,1,MPI_INT,0,MPI_SendNextEC,MPI_COMM_WORLD,MPI_STATUSES_IGNORE); //blocking wait for answer
	return EC;
}
