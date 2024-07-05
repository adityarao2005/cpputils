#include <cpputils/core/chrono.h>

cpputils::chrono::time_point cpputils::chrono::now()
{
    return std::chrono::system_clock::now();
}

double cpputils::chrono::diff(const cpputils::chrono::time_point &start, const cpputils::chrono::time_point &end)
{
    return std::chrono::duration_cast<duration>(end - start).count();
}