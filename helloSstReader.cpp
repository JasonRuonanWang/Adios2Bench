/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * helloSstReader.cpp
 *
 *  Created on: Aug 17, 2017
v *      Author: Greg Eisenhauer
 */

#include <chrono>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

#include <adios2.h>

#ifdef ADIOS2_HAVE_MPI
#include <mpi.h>
#endif

int mpiRank = 0;
int mpiSize = 1;

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

int main(int argc, char *argv[])
{

#ifdef ADIOS2_HAVE_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
#endif
    size_t r = static_cast<size_t>(mpiRank);
    size_t s = static_cast<size_t>(mpiSize);

    std::vector<size_t> start = {0, 0, 0};
    std::vector<size_t> count = {6,8,10};

    try
    {
#ifdef ADIOS2_HAVE_MPI
        adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
#else
        adios2::ADIOS adios(adios2::DebugON);
#endif

        adios2::IO &sstIO = adios.DeclareIO("myIO");
        sstIO.SetEngine("Sst");

        adios2::Engine &sstReader = sstIO.Open("helloSst", adios2::Mode::Read);
        sstReader.BeginStep();
        adios2::Variable<float> *bpFloats =
            sstIO.InquireVariable<float>("bpFloats");

        std::vector<size_t> shape = bpFloats->m_Shape;

        DumpInfo(shape, start, count);


        size_t dataSize = std::accumulate(count.begin(), count.end(), 1,
                                          std::multiplies<size_t>());

        const adios2::Box<adios2::Dims> sel(start, count);

        std::vector<float> myFloats;
        myFloats.resize(dataSize);

        bpFloats->SetSelection(sel);
        sstReader.GetDeferred(*bpFloats, myFloats.data());
        sstReader.EndStep();
        Dump(myFloats);

        sstReader.Close();
    }
    catch (std::invalid_argument &e)
    {
        std::cout << "Invalid argument exception, STOPPING PROGRAM from rank "
                  << mpiRank << "\n";
        std::cout << e.what() << "\n";
    }
    catch (std::ios_base::failure &e)
    {
        std::cout << "IO System base failure exception, STOPPING PROGRAM "
                     "from rank "
                  << mpiRank << "\n";
        std::cout << e.what() << "\n";
    }
    catch (std::exception &e)
    {
        std::cout << "Exception, STOPPING PROGRAM from rank " << mpiRank
                  << "\n";
        std::cout << e.what() << "\n";
    }

#ifdef ADIOS2_HAVE_MPI
    MPI_Finalize();
#endif

    return 0;
}
