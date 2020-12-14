PortSmashA
=

使用Arachne用户态线程库防御PortSmash攻击。

基于这项工作，我们将此作为一种独占物理核的方法，可以防范大部分超线程攻击。PortSmash只是其中一种超线程攻击案例，并且PortSmashA就是针对PortSmash的防御案例。

相关内容也可以参考《Partial-SMT: Core-scheduling Protection Against SMT Contention-based Attacks》

PortSmash
-

Aldaya A C, Brumley B B, ul Hassan S, et al. Port contention for fun and profit[C]//2019 IEEE Symposium on Security and Privacy (SP). IEEE, 2019: 870-887.

https://github.com/bbbrumley/portsmash 

攻击者登陆到受害者的兄弟核，然后通过检测兄弟核之间的运算单元时延（比如一个加法单元，A使用了加法单元，B需要执行加法，B的加法就会放到加法保留站进行排队，产生时延），来判断受害者当前执行的操作是不是某个操作（比如加法操作）。进一步可以反推控制流，比如判断if分支走的左分支或者右分支，来反推if分支判断条件，像ECC等一些加密算法会依赖于密钥bit位控制分支走向（PortSmash目前只做了时延探测，还没有试图去反推控制流，窃取秘密）。

PortSmashA
-

通过Arachne提供的用户态核调度方案，我们可以让受害者强制占据一个物理核的两个逻辑核，使得攻击者无法登陆上来，相当于把攻击者从这个物理核隔离出去了。如果受害者是一个单线程，那么我们需要额外启动一个Dummy线程来确保受害者通过两个线程完全占据物理核的兄弟核。如果受害者是多线程，比如AES的多线程模式，那么我们就可以让这个多线程来完全占据物理核的兄弟核，不需要Dummy线程了（比如一个两线程进程，直接完全占据物理核的兄弟核。奇数个线程【如3】的进程，那还是需要额外的一个Dummy线程）

Arachne
-

Arachne的性能分析结果可以参考

Qin H, Li Q, Speiser J, et al. Arachne: core-aware thread management[C]//13th {USENIX} Symposium on Operating Systems Design and Implementation ({OSDI} 18). 2018: 145-160.

https://github.com/PlatformLab/arachne-all

也可以参考我对Aes多线程模式跑在Arachne上的性能分析 https://github.com/LihengChen9/AesA 中的Excel文件，以及我的博客 https://blog.csdn.net/clh14281055/article/details/107793477 。

第0步，针对Ubuntu-20.04 gcc-9.3.0
-

编译会报错：-Werror=class-memaccess
需要针对PortSmashA/arachne-all/Arachne::Makefile::3，将
```
CXXFLAGS=-g -Wall -Werror -Wformat=2 -Wextra -Wwrite-strings \
```
改为如下，添加-Wno-error=class-memaccess
```
CXXFLAGS=-g -Wall -Werror -Wno-error=class-memaccess -Wformat=2 -Wextra -Wwrite-strings \
```
因为这是一个子模块，一开始还没下载下来。因此先调用如下，再去修改编译选项。
```
git submodule update --init --recursive
```

ubuntu-18.04没有此问题，不需要作此修改，因此这里不自动地去打patch了。

class-memaccess报错还是尽量留着好，编译的时候可以显示错误信息，这里简单的去掉了这个选项。

第1步，准备工作
-

下载所有submodules。
```
git submodule update --init --recursive
```
对Arachne进行略微修改；编译项目(包括arachne-all以及本项目)进行；关闭CPU Turbo；使CPU频率100%。提前编译Arachne是因为后续会用到Arachne进行防御。

这些步骤只需执行我准备的pre.sh（注意在项目主目录执行，不然会有路径问题）
```
shell/pre.sh
```

Demo 1 PortSmash攻击成功
-

启动受害者
```
shell/victim.sh
```
启动攻击者。X设置为受害者逻辑核ID的兄弟核。比如我的CPU是4个物理核，那么如果受害者逻辑核ID是6，那么X设置为2。
```
shell/spy.sh X
```
![image](https://github.com/LihengChen9/PortSmashA/blob/master/Demo1.png)

图中是一个大量200到240+的波动折线，有波峰有波谷。波谷是正常没有（运算单元）端口冲突的情况，200左右的时延。波峰240+是有端口冲突的时延（用红色标出），240是界定是否存在端口冲突的阈值，说明检测到了端口冲突。因此就是“有冲突”、“无冲突”、……不断交替的过程，因为ECC运算中，部分操作占据了端口5，还有一部分操作并不占据端口5，（我用的端口5去检测冲突情况，也就是用vpermd操作可以较为稳定产出攻击效果）。

240阈值的意义就好比，200是一个正常办理银行存钱业务的时间，240+中溢出的40+就是排队时长。

使用rdtsc硬件指令来读取开始时间和结束时间，求差即为时延，这里的单位为cycle。

此图说明攻击成功，因为有大于阈值的波峰（顺带标红了）

Demo 2 PortSmash攻击失败
-

启动Arachne服务器
```
sudo shell/coreArbiterServer.sh
```
启动受害者
```
shell/victim.sh
```
启动攻击者
```
shell/spy.sh X
```
此时，攻击者这边无法登陆到受害者的兄弟核上面，会报错，得不到任何时延数据。
![image](https://github.com/LihengChen9/PortSmashA/blob/master/Demo2.png)

此图，攻击者没有获得任何时延数据

Demo 3 PortSmash攻击失败
-

启动Arachne服务器
```
sudo shell/coreArbiterServer.sh
```
启动受害者
```
shell/victim.sh
```
启动攻击者。Y选择非受害者兄弟核。
```
shell/spy.sh Y
```
此时，攻击者登陆的逻辑核和受害者不是兄弟核，因此无法造成运算单元冲突，无法窃取有价值的冲突时延。
![image](https://github.com/LihengChen9/PortSmashA/blob/master/Demo3.png)

此图只有一些200的非常平稳的数据，说明没有端口冲突，没有攻击成功。（有个别异常值，是由于正常情况下，比如攻击者自身的运算间的干扰，又或者攻击者的兄弟核有其他线程的干扰。要记得此时攻击者和受害者没有登陆到同一个物理核）

扩展
-

此外，如果你有额外的兴趣，可以对shell/victim.sh里面的shell/ecc.sh 1 1这两个1参数进行修改。第一个1代表让受害者跑在Arachne线程上。第二个1代表开启Dummy线程，目的是为了让受害者完全抢占一整个物理核，如果是多线程应用，其实没必要开启Dummy线程。shell/spy.sh中，不带参数的话，spy跑在任意核心上；带参数数字如1，表示spy绑核到1；带参数A，代表spy作为一个Arachne线程运行。shell/coreArbiterServer.sh中--coresUsed 2,6参数代表目前Arachne服务器只会用到2,6逻辑核，这个可以根据你的需求修改，比如2,6,7等等（我这边有8个逻辑核）。

Demo 4 PortSmash攻击成功
-

启动Arachne服务器
```
sudo shell/coreArbiterServer.sh （包含2,6核心）
```
启动受害者
```
shell/victim.sh （关闭Dummy线程，shell/ecc.sh第二个参数置为0）
```
启动攻击者
```
shell/spy.sh X
```
由于此时受害者没有通过Dummy线程占据兄弟核，攻击者可以通过taskset抢占兄弟核来攻击。
![image](https://github.com/LihengChen9/PortSmashA/blob/master/Demo4.png)

Demo 5 PortSmash攻击成功
-

启动Arachne服务器
```
sudo shell/coreArbiterServer.sh （包含2,6核心）
```
启动受害者
```
shell/victim.sh （关闭Dummy线程，shell/ecc.sh第二个参数置为0）
```
启动攻击者
```
shell/spy.sh A (攻击者也作为一个Arachne线程)
```
由于此时受害者没有通过Dummy线程占据兄弟核，攻击者可以也作为一个Arachne线程抢占兄弟核来攻击。
![image](https://github.com/LihengChen9/PortSmashA/blob/master/Demo5.png)

Demo 6 PortSmash攻击失败
-

启动Arachne服务器
```
sudo shell/coreArbiterServer.sh （包含2,6,7核心）
```
启动受害者
```
shell/victim.sh （不关闭Dummy线程，那么受害者会优先占据2,6核心）
```
启动攻击者
```
shell/spy.sh A (攻击者也作为一个Arachne线程，此时攻击者只剩7可以占据)
```
此时，攻击者登陆的逻辑核和受害者不是兄弟核，因此无法造成运算单元冲突，无法窃取有价值的冲突时延。
![image](https://github.com/LihengChen9/PortSmashA/blob/master/Demo6.png)

小结
-

我们的防御目标是攻击者无法登陆到受害者线程的兄弟核，或者攻击者登陆到非受害者兄弟核的其他核心。
