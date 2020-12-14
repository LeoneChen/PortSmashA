# Author: Liheng Chen, ISCAS, 791960492@qq.com

make dataclean
str1=`ps -aux|grep coreArbiterServer`
str2="bin/coreArbiterServer"
if [[ $str1 =~ $str2 ]]; then
	shell/ecc.sh 1 1
else
	shell/ecc.sh 0 0
fi

