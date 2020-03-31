/*
 * FuzzyTest - simple random program generator.
 * (C) Maxim Menshikov 2019-2020
 */
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "Generator.hpp"

using namespace FuzzyTest;

#ifdef FIXED_SEED
#define SEED 0x531
#else
#define SEED (std::time(0))
#endif

int
main(int argc, const char *argv[])
{
    int       rand;
    Generator generator;

    if (argc != 2)
    {
        std::cerr << "Usage: " << std::string(argv[0]) << " path" << std::endl;
        return 1;
    }

    std::srand(SEED);
    /* Just get the gears rolling */
    std::rand();

    generator.generateTestScript(std::string(argv[1]));
    return 0;
}