//
// Created by brett on 22/11/22.
//
#include <engine/mpi.h>
#include <engine/util/std.h>

#ifdef USE_MPI
namespace Raytracing {
    void MPI::init(){
        MPI_Init(NULL, NULL);
        MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
        MPI_Comm_rank(MPI_COMM_WORLD, &currentProcessID);
        char processorName[MPI_MAX_PROCESSOR_NAME];
        int NAME_LEN_UNUSED;
        MPI_Get_processor_name(processorName, &NAME_LEN_UNUSED);
        tlog << "Starting processor " << processorName << " with an ID of " << currentProcessID << "\n";
    }
}
#endif