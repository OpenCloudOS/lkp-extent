![img](images/lkp.png)

# lkp-extent

lkp-extent项目致力于增加[LKP](https://github.com/intel/lkp-tests "LKP")项目在容器环境下的压力相关测试，从而增加Linux系统的ras特性。目前主要分为如下几个方向：
```
  1. 增加LKP远程管理分发机制 （已完成）
  2. 增加LKP对容器环境的测试 （大部分完成）
  3. 增加ebpf测试内核的用例  （待完成）
```

## 1. 项目介绍

  虽然lkp-extent是[LKP](https://github.com/intel/lkp-tests "LKP")的一个扩展功能，但是该repo并不会对[LKP](https://github.com/intel/lkp-tests "LKP")工程本身进行修改，他可以在最大的程度上使用最新的[LKP](https://github.com/intel/lkp-tests "LKP")，而lkp-extent上面新增的测试case也只会以overlay的方式叠加到原生态的LKP工程上。

  1) 远程管理与分发
  
  我们知道在测试系统稳定性的时候，对不同类型的服务器进行大批量暴力测试，才能更容易说明问题。但是原生态的[LKP](https://github.com/intel/lkp-tests "LKP")套件主要是用于在单台Linux服务器上测试系统的稳定性，如果需要部署多台不同类型的服务器进行同时测试，则需要手动登录每一台服务器进行设置。因此lkp-extent主要用于扩展[LKP](https://github.com/intel/lkp-tests "LKP")功能，解决其在这方面的不足，设计出一个 一对多点的运作模式。

                                         lkp-server
                                             |
          +-----------------+----------------+----------------+-------------------+
          |                 |                |                |                   |
          |                 |                |                |                   |
       lkp-node         lkp-node           .....           lkp-node           lkp-node

  如图，首先我们将一台服务器称为node，而lkp-extent则需要存在一个server node和若干个client node。server node会进入监听模式，client node会根据自身的服务器配置相继接入对应的server node。这样server node上面，就可以对client node进行全方位的操作.

  2) 容器环境支持

  目前[LKP](https://github.com/intel/lkp-tests "LKP")的测试case更多的是偏向于对Linux内核本身的测试，然而对于容器环境相关的测试则是几乎没有，因此lkp-extent工程会重点研发容器环境相关的测试case，用于打造一个更稳定的容器OS。
  
  3) 实现
  ```
  lkp-extent使用以下第三方库：
  （1） 基于muduo  ：满足C10K并发要求，适合内网工具使用的非阻塞异步网络库
  （2） 基于overlayfs：与原lkp项目解耦，容器挂载于overlayfs的merged目录
  （3） 基于protobuf ：高性能消息序列化工具


  lkp-extent目前提供以下能力：
  （1）对客户端的动态管理，命令下发，testcase分发，result回收。减轻测试运维压力。
  （2）对lkp的容器扩展，在容器中执行测试，将result打包至宿主机目录内。


  lkp-extent目前存在以下不足：
  （1）缺少对运行lkp的docker的后台管理，后续可以考虑加入。
  （2）缺少对docker特性的lkp测试用例，后续可以考虑加入。
  （3）目前的docker镜像使用ubuntu制作。可以制作OpenCloudOS版本镜像用于测试，后续可以考虑加入其他docker镜像。
  ```
  
## 2. 安装方法
  2.1 安装TencentOS或者OpenCloudOS镜像

  2.2 安装编译需要的相关依赖
  ```
  yum install boost-devel cmake
  ```
  2.2 protobuf安装（使用protobuf 3.0.0离线压缩包，在etc目录下有安装包）
  ```
  ./configure --prefix=/usr/`
  make
  make install
  ```
  
  2.3 lkp-extent安装
  
  ```
  git clone https://github.com/OpenCloudOS/lkp-extent.git
  ./build.sh     # 编译lkp-extent代码
  ```
  
  输入lkp-ctl可以看到lkp-extent的介绍则说明安装成功。
 
## 3. 使用方法

  **使用之前需要在lkp-extent.config配置文件中修改IP、port等配置。**
  
（1）初始化，下载安装lkp项目，配置overlayfs并mount。
  ```
  lkp-ctl init
  ```

（2）开启服务，可选择为当前主机开启lkp-extent的client或者server服务
  ```
  lkp-ctl start server  
  lkp-ctl start client
  ```
    
（3）显示当前已经连接的节点数量与信息，只对server有效
  ```
  lkp-ctl list
  ```
    
（4）控制节点执行lkp测试
  ```
  lkp-ctl run ebizzy                  #执行本机lkp测试  
  lkp-ctl -a run ebizzy               #所有远程client节点执行ebizzy测试   
  lkp-ctl -i 2 -c 5 dockertest        #2号节点开启5个容器执行dockertest测试（当前没有dockertest.yaml，使用ebizzy进行过容器测试）
  ```
  
（5）向远端节点推送自定义testcase
  ```
  lkp-ctl -a push dockertest.yaml     #向所有远端节点广播推送  
  lkp-ctl -i 0 push dockertest.yaml   #向0号节点推送dockertest.yaml，会添加到/lkp-tests/jobs中
  ```
    
（6）从远端节点回收测试结果
  ```
  lkp-ctl -a result                   #向所有节点回收结果
  lkp-ctl -i 0 result                 #向0号节点回收结果
  ```
    
（7）命令远端节点更新
  ```
  lkp-ctl -a update                   #所有节点更新
  lkp-ctl -i 11 update                #11号节点更新
  ```
    
（8）关闭lkp-extent服务
  ```
  lkp-ctl stop
  ```
  
  ## 4. 结语
  lkp-extent是一个依赖于LKP测试套件的工程，因此lkp-extent工程里面关于容器测试case的写法和[LKP](https://github.com/intel/lkp-tests "LKP")的job写法相同([readme](https://github.com/intel/lkp-tests/blob/master/doc/add-testcase.md "how to"))。在使用过程中，lkp-extent会将自身的测试项overlay到[LKP](https://github.com/intel/lkp-tests "LKP")工程的jobs目录上，这样方便LKP工程进行统一管理。同样在之前提到的，拥有远程管理的基础上，lkp-ctl可以拥有通过配置文件，让多个host在指定时间，指定条件进行自动部署，自动测试和自动返回测试结果的功能。
  
