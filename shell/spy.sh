# Author: Liheng Chen, ISCAS, 791960492@qq.com

# Spy: must be on same physical core, but different logical core start spying and generate the message to be signed

pkill spy

if [ ! -n "$1" ]; then
  bin/spy
elif [ "$1" == "A" ]; then
  bin/spy 1
else
  taskset -c "$1" bin/spy
fi

python3 py/parse_raw_simple.py bin/timings.bin
