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

#include <openssl/obj_mac.h>
#include <openssl/ec.h>

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Arachne/Arachne.h"
#include "PortSmashA/tools.h"
#include "PortSmashA/spy.h"

void dummy(int argc, const char **argv) {
//    printf("[Dummy] pid: %d, tid: %ld, Cores Affinity: %d(%s)\n", getpid(), syscall(SYS_gettid), sched_getcpu(),
//           getaffinity_str().c_str());
}

void ecc(int argc, const char **argv) {
    printf("[Ecc] pid: %d, tid: %ld, Cores Affinity: %d(%s)\n", getpid(), syscall(SYS_gettid), sched_getcpu(),
           getaffinity_str().c_str());
    if (argc < 4 || (strcmp(argv[1], "A") && strcmp(argv[1], "D") && strcmp(argv[1], "AD") && strcmp(argv[1], "M"))) {
        printf("usage: %s <A|D|M> <iterations> <nonce> /* (A)dd (D)ouble (M)ultiply */\n", argv[0]);
        return;
    }

    /* number of iterations */
    int its = atoi(argv[2]);
    /* create new CTX*/
    BN_CTX *ctx = BN_CTX_new();
    assert(ctx != NULL);
    BN_CTX_start(ctx);

    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_secp384r1);
    //EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_secp521r1);
    //EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_brainpoolP512r1);
    assert(group != NULL);

    /* initialize variables */
    BIGNUM *k, *order, *x, *y;
    k = BN_CTX_get(ctx);
    order = BN_CTX_get(ctx);
    x = BN_CTX_get(ctx);
    y = BN_CTX_get(ctx);
    assert(y != NULL);

    /* initialize EC points */
    EC_POINT *Q = EC_POINT_new(group);
    EC_POINT *P = EC_POINT_new(group);
    assert(Q != NULL);
    assert(P != NULL);

    EC_GROUP_get_order(group, order, ctx);

    /* receive nonce from cmd, generate random nonce if not provided */
    BN_hex2bn(&k, argv[3]);

    if (BN_is_zero(k))
        BN_rand_range(k, order);

    /* compute [P]k and copy to Q */
    EC_POINT_mul(group, P, k, NULL, NULL, ctx);
    EC_POINT_make_affine(group, P, ctx);
    EC_POINT_copy(Q, P);

    /* block computation using a pipe until receive from spy -- to sync victim and spy  */
    size_t ret;
    uint8_t *zeroes = (uint8_t *) calloc(ZERO_COUNT, sizeof(uint8_t));
    assert(zeroes != NULL);
    FILE *pipe;
    pipe = fopen(SPY_PIPE, "rb");
    assert(pipe != NULL);
    ret = fread(zeroes, sizeof(uint8_t), ZERO_COUNT, pipe);
    assert(ret == ZERO_COUNT);
    fclose(pipe);
    free(zeroes);

    /* perform Add and Double */
    if (!strcmp(argv[1], "AD")) {
        for (; its; its--) {
            EC_POINT_add(group, Q, Q, P, ctx);
        }
        for (; its; its--) {
            EC_POINT_dbl(group, Q, Q, ctx);
        }
    }
        /* perform only ADD or DOUBLE or MULTIPLY */
    else if (!strcmp(argv[1], "A")) {
        for (; its; its--) {
            EC_POINT_add(group, Q, Q, P, ctx);
        }
    } else if (!strcmp(argv[1], "D")) {
        for (; its; its--) {
            EC_POINT_dbl(group, Q, Q, ctx);
        }
    } else if (!strcmp(argv[1], "M")) {
        for (; its; its--) {
            EC_POINT_mul(group, Q, k, NULL, NULL, ctx);
        }
    } else {
        assert(0);
    }

    /* get x and y coordinates from Q */
    EC_POINT_get_affine_coordinates_GFp(group, Q, x, y, ctx);
    char *s0 = BN_bn2hex(k);
    char *s1 = BN_bn2hex(x);
    char *s2 = BN_bn2hex(y);
    /* print to console */
    //printf(" k: 0x%s\n", s0);
    //printf("Px: 0x%s\n", s1);
    //printf("Py: 0x%s\n", s2);

    /* free memory */
    BN_CTX_end(ctx);
    EC_POINT_free(P);
    EC_POINT_free(Q);
    EC_GROUP_free(group);
    BN_CTX_free(ctx);

    free(s0);
    free(s1);
    free(s2);

}

int main(int argc, const char **argv) {
//    printf("[Ecc Main] pid: %d, tid: %ld, Cores Affinity: %d(%s)\n", getpid(), syscall(SYS_gettid), sched_getcpu(),
//           getaffinity_str().c_str());
    bool EccOnArachneFlag = !strcmp(argv[1], "1");
    bool DummyFlag = !strcmp(argv[2], "1");

    memmove(argv + 1, argv + 3, (argc - 3) * sizeof(char *));
    argc -= 2;

    if (EccOnArachneFlag) {
        Arachne::Logger::setLogLevel(Arachne::ERROR);
        Arachne::init(&argc, argv);
        auto t_ecc = Arachne::createThread(&ecc, argc, argv);
        if (DummyFlag) {
            auto t_dummy = Arachne::createThread(&dummy, argc, argv);
            Arachne::join(t_dummy);
        }
        Arachne::join(t_ecc);
        Arachne::shutDown();
        Arachne::waitForTermination();
    } else {
        ecc(argc, argv);
    }
}
