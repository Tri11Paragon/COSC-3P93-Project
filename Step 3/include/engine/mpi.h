//
// Created by brett on 22/11/22.
//

#ifndef STEP_3_MPI_H
#define STEP_3_MPI_H

#include <config.h>

#ifdef USE_MPI

#include <mpi.h>
#include <queue>
#include <engine/raytracing.h>

namespace Raytracing {
    
    extern int numberOfProcesses;
    extern int currentProcessID;
    
    class MPI {
        public:
            static void init(int argc, char** argv);
            static std::queue<RaycasterImageBounds> getCurrentImageRegionAssociation(RayCaster& raycaster);
    };
}
#endif

#endif //STEP_3_MPI_H
