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

    GenData(true, vars, count);

    adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
    auto adiosIO = adios.DeclareIO("myIO");
    adiosIO.SetEngine(engine);
    adiosIO.SetParameters(engineParams);
    auto adiosEngine = adiosIO.Open("AdiosBench", adios2::Mode::Read);

    for(size_t t=0; t<steps; ++t)
    {
        adiosEngine.BeginStep();
        for(size_t i=0; i<vars; ++i){
            std::string varName = "floatsVar" + std::to_string(i);
            auto variable = adiosIO.InquireVariable<float>(varName);
            if(!variable)
            {
                std::cout << "bpFloats variable is nullptr\n";
            }
            std::pair<float, float> minmax = variable.MinMax();
            std::cout << "min = " << minmax.first << " max = " << minmax.second << std::endl;
            variable.SetSelection({start, count});
            adiosEngine.Get(variable, floatsVecVec[i].data());
        }
        adiosEngine.EndStep();
    }

    adiosEngine.Close();
    MPI_Finalize();

    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> duration = end - begin;
    std::cout << duration.count() << std::endl;

    return 0;
}
