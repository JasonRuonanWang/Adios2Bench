#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

#include "common.h"

int main(int argc, char *argv[])
{

    adios2::Dims shape = {10, 10};
    adios2::Dims start = {0, 0};
    adios2::Dims count = {10, 10};
    size_t vars = 1;
    std::string engine = "Wdm";
    adios2::Params engineParams = {{"Verbose","11"}};

    auto begin = std::chrono::system_clock::now();
    writer(shape, start, count, vars, engine, engineParams, 1000000);
    auto end = std::chrono::system_clock::now();

    return 0;
}
