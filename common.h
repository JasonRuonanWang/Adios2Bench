#include <vector>
#include <adios2.h>
#ifdef ADIOS2_HAVE_MPI
#include <mpi.h>
#endif


int mpiRank = 0;
int mpiSize = 1;


std::vector<std::vector<float>> floatsVecVec;
std::vector<adios2::Variable<float>> floatsVarVec;

void GenData(const bool zero, const size_t pVars, const adios2::Dims &count){

    size_t r = static_cast<size_t>(mpiRank);
    size_t s = static_cast<size_t>(mpiSize);
    size_t datasize = std::accumulate(count.begin(), count.end(), 1, std::multiplies<size_t>());

    for(size_t i=0; i<pVars; ++i){
        floatsVecVec.emplace_back();
        floatsVecVec.back().resize(datasize);
        if(!zero){
            for(size_t j=0; j<datasize; ++j){
                floatsVecVec.back()[j]=j;
            }
        }
    }
}

template <class T>
void Dump(std::vector<T> &v)
{
    for (int i = 0; i < mpiSize; ++i)
    {
        MPI_Barrier(MPI_COMM_WORLD);
        if (mpiRank == i)
        {
            std::cout << "Dumping data from Rank " << mpiRank << ": "
                      << std::endl;
            for (const auto &i : v)
            {
                std::cout << i << " ";
            }
            std::cout << std::endl;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

void DumpInfo(std::vector<size_t> &shape, std::vector<size_t> &start, std::vector<size_t> &count){

    for (int i = 0; i < mpiSize; ++i)
    {
        MPI_Barrier(MPI_COMM_WORLD);
        if (mpiRank == i)
        {
            std::cout <<"===========================" << std::endl;
            std::cout << "Dumping info from Rank " << mpiRank << std::endl;
            std::cout << "shape = [";
            for (auto &i : shape)
            {
                std::cout << i << ", ";
            }
            std::cout << "]" << std::endl;

            std::cout << "start = [";
            for (auto &i : start)
            {
                std::cout << i << ", ";
            }
            std::cout << "]" << std::endl;

            std::cout << "count = [";
            for (auto &i : count)
            {
                std::cout << i << ", ";
            }
            std::cout << "]" << std::endl;
            std::cout <<"===========================" << std::endl;
        }
    }
}

void reader(
        const adios2::Dims &pShape,
        const adios2::Dims &pStart,
        const adios2::Dims &pCount,
        size_t pVars,
        const std::string &pEngine,
        const adios2::Params &pEngineParams,
        const size_t steps
        ){

    MPI_Init(0, 0);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

    GenData(true, pVars, pCount);

    adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
    auto adiosIO = adios.DeclareIO("myIO");
    adiosIO.SetEngine(pEngine);
    adiosIO.SetParameters(pEngineParams);
    auto adiosEngine = adiosIO.Open("AdiosBench", adios2::Mode::Read);

    for(size_t t=0; t<steps; ++t)
    {
        adiosEngine.BeginStep();
        for(size_t i=0; i<pVars; ++i){
            std::string varName = "floatsVar" + std::to_string(i);
            std::cout << " before var !!!!!!\n";
            floatsVarVec[i] = adiosIO.InquireVariable<float>(varName);
            std::cout << " after var !!!!!!\n";
            if(!floatsVarVec[i])
            {
                std::cout << "bpFloats variable is nullptr\n";
            }
//            std::pair<float, float> minmax = bpFloats.MinMax();
//            std::cout << "min = " << minmax.first << " max = " << minmax.second << std::endl;
//            bpFloats.SetSelection({pStart, pCount});
            std::cout << "before\n";
            adiosEngine.Get(floatsVarVec[i], floatsVecVec[i].data());
            std::cout << "after\n";
        }
        adiosEngine.EndStep();
    }

    adiosEngine.Close();
    MPI_Finalize();
}

void writer(
        const adios2::Dims &pShape,
        const adios2::Dims &pStart,
        const adios2::Dims &pCount,
        size_t pVars,
        const std::string &pEngine,
        const adios2::Params &pEngineParams,
        const size_t steps
        ){


    MPI_Init(0, 0);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

    GenData(false, pVars, pCount);

    adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
    auto adiosIO = adios.DeclareIO("myIO");
    adiosIO.SetEngine(pEngine);
    adiosIO.SetParameters(pEngineParams);
    auto adiosEngine = adiosIO.Open("AdiosBench", adios2::Mode::Write);

    for(size_t i=0; i<pVars; ++i){
        std::string varName = "floatsVar" + std::to_string(i);
        floatsVarVec.emplace_back(adiosIO.DefineVariable<float>(varName, pShape, pStart, pCount));
    }

    for(size_t t=0; t<steps; ++t)
    {
        adiosEngine.BeginStep();
        for(size_t i=0; i<pVars; ++i){
            adiosEngine.Put(floatsVarVec[i], floatsVecVec[i].data());
        }
        adiosEngine.EndStep();
    }


    adiosEngine.Close();

    MPI_Finalize();
}
