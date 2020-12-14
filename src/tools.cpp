//
// Author: Liheng Chen, ISCAS
//
#include <unistd.h>
#include "PortSmashA/tools.h"

std::string getaffinity_str() {
    /* 查看当前线程所运行的所有cpu*/
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    std::string affinity_str = "";
    pthread_getaffinity_np(pthread_self(), sizeof(cpus), &cpus);
    for (int i = 0; i < sysconf(_SC_NPROCESSORS_CONF); i++) {
        if (CPU_ISSET(i, &cpus))
            affinity_str += std::to_string(i) + ",";
    }
    return affinity_str;
}