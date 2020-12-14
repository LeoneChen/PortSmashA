# Author: Liheng Chen, ISCAS, 791960492@qq.com
# Download All submodules
# git submodule update --init --recursive

# Some Arachne's Modification
cd arachne-all/Arachne/src
mkdir -p ../include/Arachne
cp *.h ../include/Arachne

cd ../../ArachnePerfTests/src
mkdir -p ../include/ArachnePerfTests
cp *.h ../include/ArachnePerfTests

cd ../../CoreArbiter/src
mkdir -p ../include/CoreArbiter
cp *.h ../include/CoreArbiter

cd ../../PerfUtils/src
mkdir -p ../include/PerfUtils
cp *.h ../include/PerfUtils

# install some things
sudo apt install build-essential libpcre3-dev libssl-dev python3 python3-pip
pip3 install scipy matplotlib

# compile arachne-all
cd ../../
./cleanAll.sh
./buildAll.sh
cd ..

# make CPU Frequency 100%
echo "1" | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo > /dev/null
echo "performance" | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor > /dev/null

# compile this project
make deepclean
make -j`nproc`
