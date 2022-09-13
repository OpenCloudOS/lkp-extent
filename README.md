![img](images/lkp.png)

# lkp-extent
lkp-extent项目致力于增加[LKP](https://github.com/intel/lkp-tests "LKP")项目在容器环境下的压力相关测试，从而增加Linux系统的ras特性。目前主要分为如下几个方向：
1. 增加LKP远程管理分发机制
2. 增加LKP对容器环境的测试
3. 增加ebpf测试内核的用例

## 进展
**此分支已完成一个基于muduo的多线程服务器，lkp-server可以远程控制连接上的lkp-node执行lkp测试。已测试支持命令如下一小节所示。源代码存储在/src中。**

**项目正在完善lkp-extent.config\makefile\init.sh等部分...**

## 远程管理
#### 介绍
我们知道在测试系统稳定性的时候，对不同类型的服务器进行大批量暴力测试，才能更容易说明问题。但是原生态的[LKP](https://github.com/intel/lkp-tests "LKP")套件主要是用于在单台Linux服务器上测试系统的稳定性，如果需要部署多台不同类型的服务器进行同时测试，则需要手动登录每一台服务器进行设置。因此lkp-extent主要用于扩展[LKP](https://github.com/intel/lkp-tests "LKP")功能，解决其在这方面的不足，设计出一个 一对多点的运作模式。

#### 原理
                                         lkp-server
                                             |
          +-----------------+----------------+----------------+-------------------+
          |                 |                |                |                   |
          |                 |                |                |                   |
       lkp-node         lkp-node           .....           lkp-node           lkp-node

如图，首先我们将一台服务器称为node，而lkp-extent则需要存在一个server node和若干个client node。server node会进入监听模式，client node会根据自身的服务器配置相继接入对应的server node。这样server node上面，就可以对client node进行全方位的操作.

#### 使用方法
 1.  查看有哪些待测服务器

      `$ lkp-ctl list` # 返回client-node的nodeid，arch类型和内存等信息

 2. 更新测试组件

    `$ lkp-ctl update` # 更新本机的测试组件，包括lkp-extent和lkp两个repo

    `$ lkp-ctl update -a` # 更新所有node的测试组件

    `$ lkp-ctl update -i nodeid` #更新id号是nodeid的node的测试组件

 3. 下发测试指令

    `$ lkp-ctl run testcase` # 本机执行testcase
    
    `$ lkp-ctl run testcase -a ` # 所有远程node执行testcase

    `$ lkp-ctl run testcase -i nodeid ` # id是nodeid的服务器执行testcase

    `$ lkp-ctl run testcase -i nodeid -c containerCount` # id是nodeid的服务器开containerCount个容器，并在容器里面执行testcase


 4. 推送自定义测试case

    `$ lkp-ctl push testscript -a` # 向所有测试机推送名字为testscript的脚本

    `$ lkp-ctl push testscript -i nodeid ` # 向指定测试机推送名字为testscript的脚本

 5. 收集测试结果

    `$ lkp-ctl result -a` # 收集所有测试机的测试结果

    `$ lkp-ctl result -i nodeid ` # 收集指定的测试机的测试结果

虽然lkp-extent是[LKP](https://github.com/intel/lkp-tests "LKP")的一个扩展功能，但是该repo并不会对[LKP](https://github.com/intel/lkp-tests "LKP")工程本身进行修改，他可以在最大的程度上使用最新的[LKP](https://github.com/intel/lkp-tests "LKP")，而lkp-extent上面新增的测试case也只会以overlay的方式叠加到原生态的LKP工程上。

## 容器测试
#### 介绍
目前[LKP](https://github.com/intel/lkp-tests "LKP")的测试case更多的是偏向于对Linux内核本身的测试，然而对于容器环境相关的测试则是几乎没有，因此lkp-extent工程会重点研发容器环境相关的测试case，用于打造一个更稳定的容器OS。

#### 原理
lkp-extent是一个依赖于[LKP](https://github.com/intel/lkp-tests "LKP")测试套件的工程，因此lkp-extent工程里面关于容器测试case的写法和[LKP](https://github.com/intel/lkp-tests "LKP")的job写法相同([readme](https://github.com/intel/lkp-tests/blob/master/doc/add-testcase.md "how to"))。在使用过程中，lkp-extent会将自身的测试项overlay到[LKP](https://github.com/intel/lkp-tests "LKP")工程的jobs目录上，这样方便LKP工程进行统一管理。同样在之前提到的，拥有远程管理的基础上，lkp-ctl可以拥有通过配置文件，让多个host在指定时间，指定条件进行自动部署，自动测试和自动返回测试结果的功能。

#### 进展
正在研发中......


