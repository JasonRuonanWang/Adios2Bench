#include <vector>
#include <adios2.h>
#ifdef ADIOS2_HAVE_MPI
#include <mpi.h>
#endif


adios2::Dims shape = {10, 10};
adios2::Dims start = {0, 0};
adios2::Dims count = {10, 10};

size_t steps = 10000;
size_t vars = 1;

std::string engine = "Wdm";
adios2::Params engineParams = {{"Verbose","11"}};

int mpiRank = 0;
int mpiSize = 1;

std::vector<std::vector<float>> floatsVecVec;

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

