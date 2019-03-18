#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

#include "common.h"

int main(int argc, char *argv[])
{

    std::vector<adios2::Variable<float>> floatsVarVec;

    auto begin = std::chrono::system_clock::now();

    MPI_Init(0, 0);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

    adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
    auto adiosIO = adios.DeclareIO("myIO");
    adiosIO.SetEngine(engine);
    adiosIO.SetParameters(engineParams);
    auto adiosEngine = adiosIO.Open("AdiosBench", adios2::Mode::Write);

    for(size_t i=0; i<vars; ++i){
        std::string varName = "floatsVar" + std::to_string(i);
        floatsVarVec.emplace_back(adiosIO.DefineVariable<float>(varName, shape, start, count));
    }

    for(size_t t=0; t<steps; ++t)
    {
        GenData(false, vars, count, t);
        adiosEngine.BeginStep();
        std::cout << " ===================== " << std::endl;
        std::cout << "Step " << adiosEngine.CurrentStep() << " begin" << std::endl;
        for(size_t i=0; i<vars; ++i){
            adiosEngine.Put(floatsVarVec[i], floatsVecVec[i].data());
            Dump(floatsVecVec[i], t);
        }
        adiosEngine.EndStep();
    }

    adiosEngine.Close();

    MPI_Finalize();

    auto end = std::chrono::system_clock::now();

    return 0;
}
