# Author: Liheng Chen, ISCAS, 791960492@qq.com
cd arachne-all/CoreArbiter/||exit
make -j"$(nproc)" -s
./bin/coreArbiterServer --coresUsed 2,6

