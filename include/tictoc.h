#pragma once
#include <omp.h>

double __t__t;
void tic() {
    __t__t = omp_get_wtime();
}
void toc(std::string name = "") {
    std::cout << name << "Time taken: " << omp_get_wtime()-__t__t << "s" << std::endl;
}