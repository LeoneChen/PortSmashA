# Author: Liheng Chen, ISCAS, 791960492@qq.com

OPENSSL=openssl
KEY_CURVE=secp384r1
KEY_FILE=$KEY_CURVE.pem

# create a P-384 key pair if it does not exist
if [ ! -f bin/$KEY_FILE ]; then
  $OPENSSL ecparam -genkey -name $KEY_CURVE -out bin/$KEY_FILE
  $OPENSSL ec -in bin/$KEY_FILE -pubout >>bin/$KEY_FILE
fi

# create pipe
mkfifo bin/pipe.fifo

# Victim: start scalar multiplication but it will be blocked
if [ "$2" = 1 ];then
    bin/ecc "$1" "$2" --minNumCores 2 M 4 000084210000842100008421000084210000842100008421000084210000842100008421000084210000842100008421
else
    bin/ecc "$1" "$2" M 4 000084210000842100008421000084210000842100008421000084210000842100008421000084210000842100008421
fi
