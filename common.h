#include <vector>
#include <adios2.h>
#ifdef ADIOS2_HAVE_MPI
#include <mpi.h>
#endif


int mpiRank = 0;
int mpiSize = 1;

adios2::Dims shape;
adios2::Dims start;
adios2::Dims count;

std::vector<std::vector<float>> floatsVecVec;
std::vector<adios2::Variable<float>> floatsVarVec;
size_t vars;

void GenData(bool zero){

    size_t r = static_cast<size_t>(mpiRank);
    size_t s = static_cast<size_t>(mpiSize);
    size_t datasize = std::accumulate(count.begin(), count.end(), 1, std::multiplies<size_t>());

    for(size_t i=0; i<vars; ++i){
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
        const adios2::Params &pEngineParams
        ){

    MPI_Init(0, 0);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

    GenData(true);

    adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
    auto adiosIO = adios.DeclareIO("myIO");
    adiosIO.SetEngine(pEngine);
    adiosIO.SetParameters(pEngineParams);
    auto adiosEngine = adiosIO.Open("AdiosBench", adios2::Mode::Read);

    adiosEngine.BeginStep();
    for(size_t i=0; i<vars; ++i){
        std::string varName = "floatsVar" + std::to_string(i);
        auto bpFloats = adiosIO.InquireVariable<float>(varName);
        bpFloats.SetSelection({start, count});
        adiosEngine.Get(floatsVarVec[i], floatsVecVec[i].data());
    }
    adiosEngine.EndStep();
    adiosEngine.Close();
    MPI_Finalize();
}

void writer(
        const adios2::Dims &pShape,
        const adios2::Dims &pStart,
        const adios2::Dims &pCount,
        size_t pVars,
        const std::string &pEngine,
        const adios2::Params &pEngineParams
        ){

    shape = pShape;
    start = pStart;
    count = pCount;
    vars = pVars;

    MPI_Init(0, 0);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

    GenData(false);

    adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
    auto adiosIO = adios.DeclareIO("myIO");
    adiosIO.SetEngine(pEngine);
    adiosIO.SetParameters(pEngineParams);
    auto adiosEngine = adiosIO.Open("AdiosBench", adios2::Mode::Write);

    for(size_t i=0; i<vars; ++i){
        std::string varName = "floatsVar" + std::to_string(i);
        floatsVarVec.emplace_back(adiosIO.DefineVariable<float>(varName, shape, start, count));
    }

    adiosEngine.BeginStep();
    for(size_t i=0; i<vars; ++i){
        adiosEngine.Put(floatsVarVec[i], floatsVecVec[i].data());
    }
    adiosEngine.EndStep();
    adiosEngine.Close();

    MPI_Finalize();
}
