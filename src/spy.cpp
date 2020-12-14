
/*
 *   Author: Liheng Chen, ISCAS
 *
 *   Below is PortSmash Author info
 *
 *   Copyright 2018-2019 Alejandro Cabrera Aldaya, Billy Bob Brumley, Sohaib ul Hassan, Cesar Pereida García and Nicola Tuveri
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "Arachne/Arachne.h"
#include "../include/PortSmashA/spy.h"
#include "../include/PortSmashA/tools.h"

int spy() {
    printf("[Spy] pid: %d, tid: %ld, Cores Affinity: %d(%s)\n", getpid(), syscall(SYS_gettid), sched_getcpu(),
           getaffinity_str().c_str());
    size_t ret;
    /* pipe */
    uint8_t *zeroes = (uint8_t *) calloc(ZERO_COUNT, sizeof(uint8_t));
    FILE *pipe;
    pipe = fopen(SPY_PIPE, "wb+");
    assert(pipe != NULL);
    ret = fwrite(zeroes, sizeof(uint8_t), ZERO_COUNT, pipe);
    assert(ret == ZERO_COUNT);

    fclose(pipe);
    free(zeroes);

    /* spy */
    /* size_t ret; */
    ret = 0;
    uint64_t *timings = (uint64_t *) calloc(SPY_NUM_TIMINGS, sizeof(uint64_t));
    assert(timings != NULL);

    /* call function in assembler */
    x64_portsmash_spy(timings);

    /* open file */
    FILE *fp;
    fp = fopen("bin/timings.bin", "wb+");
    assert(fp != NULL);

    ret = fwrite(timings, sizeof(uint64_t), SPY_NUM_TIMINGS, fp);
    assert(ret == SPY_NUM_TIMINGS);

    fclose(fp);
    free(timings);

    return 0;
}


int main(int argc, const char **argv) {
//    printf("[Spy Main] pid: %d, tid: %ld, Cores Affinity: %d(%s)\n", getpid(), syscall(SYS_gettid), sched_getcpu(),
//           getaffinity_str().c_str());
    bool SpyOnArachneFlag;
    if (argc > 1) {
        SpyOnArachneFlag = !strcmp(argv[1], "1");
        memmove(argv + 1, argv + 2, (argc - 2) * sizeof(char *));
        argc -= 1;
    } else {
        SpyOnArachneFlag = 0;
    }

    if (SpyOnArachneFlag) {
        Arachne::Logger::setLogLevel(Arachne::ERROR);
        Arachne::init(&argc, argv);
        auto t_spy = Arachne::createThread(&spy);
        Arachne::join(t_spy);
        Arachne::shutDown();
        Arachne::waitForTermination();
    } else {
        spy();
    }
}
