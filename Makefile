$(shell mkdir -p bin)
OPENSSL_ROOT_DIR := /usr/local/ssl

ARACHNE_INCLUDE_PATHS := -Iarachne-all/Arachne/include -Iarachne-all/CoreArbiter/include -Iarachne-all/PerfUtils/include
PORTSMASHA_INCLUDE_PATHS := -Iinclude $(ARACHNE_INCLUDE_PATHS)
CXX_FLAGS := -std=c++11 -g -Wall $(PORTSMASHA_INCLUDE_PATHS)

ARACHNE_LIB_PATHS := -Larachne-all/Arachne/lib -Larachne-all/CoreArbiter/lib -Larachne-all/PerfUtils/lib/
ARACHNE_LIB_NAMES := -lArachne -lCoreArbiter -lPerfUtils
ARACHNE_LINK_FLAGS := $(ARACHNE_LIB_PATHS) $(ARACHNE_LIB_NAMES) -lpcrecpp -pthread

all: spy ecc

spy: src/spy.cpp src/spy.S src/tools.cpp
	@g++ -o bin/$@ $^ $(CXX_FLAGS) $(ARACHNE_LINK_FLAGS)

ecc: src/ecc.cpp src/tools.cpp
	@g++ -o bin/$@ $^ $(CXX_FLAGS) $(ARACHNE_LINK_FLAGS) -lcrypto

.PHONY: clean dataclean deepclean
clean:
	@rm -f bin/spy bin/ecc bin/victim

dataclean:
	@rm -f bin/timings.bin bin/secp384r1.pem bin/pipe.fifo

deepclean: clean dataclean



