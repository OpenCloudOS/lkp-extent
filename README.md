# lkp-extent



lkp-extent是一款基于[lkp-tests](https://github.com/intel/lkp-tests)的开源测试框架，为其增加了测试集群管理、测试用例分发以及容器环境测试的能力，从而提高Linux系统的RAS特性。lkp-extent并不会对[lkp-tests](https://github.com/intel/lkp-tests)工程本身进行修改，它可以在最大的程度上使用最新的[lkp-test](https://github.com/intel/lkp-tests)。



#### 测试集群管理

> 我们知道在测试系统稳定性的时候，对不同类型的服务器进行大批量暴力测试，才能更容易说明问题。但是原生态的[lkp-tests](https://github.com/intel/lkp-tests)套件主要是用于在单台Linux服务器上测试系统的稳定性，如果需要部署多台不同类型的服务器进行同时测试，则需要手动登录每一台服务器进行设置。因此，lkp-extent解决[lkp-tests](https://github.com/intel/lkp-tests)在这方面的不足，设计出一个一对多点的运作模式。

```
                                      lkp-ctl
                                         |                                     
                                     lkp-master
                                         |
      +-----------------+----------------+----------------+-------------------+
      |                 |                |                |                   |
      |                 |                |                |                   |
   lkp-node         lkp-node           .....           lkp-node           lkp-node
```



> 如图，首先我们将一台服务器称为node，而lkp-extent则需要存在一个 node和若干个client node。server node会进入监听模式，client node会根据自身的服务器配置相继接入对应的server node。这样server node上面，就可以对client node进行全方位的操作.
>
> 如图所示，lkp-master是负责整个测试集群管理的服务节点，启动后会进入监听模式；lkp-node会根据自身配置相继介入到lkp-master上。lkp-master对上层lkp-ctl提供命令接口服务；对下层lkp-node进行资源监控以及指令控制（包括分发测试用例以及下达执行测试任务等）。lkp-ctl是一个CLI工具，通常跟lkp-master部署在同一台服务器，测试人员可以通过lkp-ctl向lkp-master下达指令。



### 教程

#### 构建

- [lkp-extent构建](./doc/build/lkp-extent构建.md)



#### 部署

- [lkp-ctl部署](./doc/deploy/lkp-ctl部署.md)

- [lkp-master部署](./doc/deploy/lkp-master部署.md)
- [lkp-node部署](./doc/deploy/lkp-node部署.md)



#### 快速开始

- [概述](./doc/quick_start/overview.md)
- [使用lkp-extent执行一次集群测试任务](./doc/quick_start/quick_start.md)

- 更多教程



### 文档

#### lkp-extent设计

- 架构
- 组件



#### 开发资源

- API
- Test



#### 欢迎您的意见

![1](image/1.jpg)