#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

#include "common.h"

int main(int argc, char *argv[])
{

    Dims shape = {10, 10};
    Dims start = {0, 0};
    Dims count = {10, 10};
    size_t vars = 1;
    string engine = "DataMan";
    Params engineParams = {{"WorkflowMode","P2P"}};

    auto begin = chrono::system_clock::now();
    writer(shape, start, count, vars, engine, engineParams);
    auto end = chrono::system_clock::now();

    return 0;
}
