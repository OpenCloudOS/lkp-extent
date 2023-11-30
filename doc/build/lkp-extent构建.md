#### 搭建lkp-extent编译环境

推荐使用容器构建项目。相较于传统的编译方式，适用容器构建项目能够保证编译环境的一致性，且多编译器版本切换更加方便。在OpencloudOS环境中默认使用podman容器引擎，这里我们使用podman-compose容器编排工具搭建lkp-extent的编译环境：

```shell
yum -y install podman podman-compose
```



安装golang 1.19的docker-compose.yaml文件：

```yaml
version: '3'

services:
  golang:
    image: golang:1.19
    container_name: golang1_19

    stdin_open: true
    tty: true

    entrypoint: ["bash"]
    working_dir: /go/

    environment:
      - GOPATH=/go/
    volumes:
      - /data/go:/go/
```

启动编译容器：

```shell
podman-compose up -d
```



#### 编译lkp-extent项目

进入编译容器：

```shell
podman exec -it golang1_19 bash
```

拉取lkp-extent源码：

```shell
git clone https://github.com/AntiBargu/lkp-extent.git
```

lkp-extent项目由三个可执行程序组成，分别是：lkp-ctl、lkp-master以及lkp-node。

编译完整的lkp-extent项目：

```shell
# 1.编译生成所有的二进制文件
make

# 2.编译生成可执行程序的安装包
make release
```

如果想独立地构建某一个可执行程序（安装包），进入相应的目录，执行make指令即可。