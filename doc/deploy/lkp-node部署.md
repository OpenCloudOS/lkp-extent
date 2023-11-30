### lkp-node部署

#### 部署前环境检查

- 主机需要能够访问外部网络，从公共网络中拉取安装包和docker镜像。



#### 构建lkp-extent

请参照[lkp-extent构建](../build/lkp-extent构建.md)



#### 下载lkp-extent-node二进制包

```shell
wget https://github.com/AntiBargu/lkp-extent/releases/download/${LKP_EXTENT_VERSION}/lkp-extent-${LKP_EXTENT_VERSION}-node.tar.gz
```

样例：

```shell
export LKP_EXTENT_VERSION=v0.2.1
wget https://github.com/AntiBargu/lkp-extent/releases/download/${LKP_EXTENT_VERSION}/lkp-extent-${LKP_EXTENT_VERSION}-node.tar.gz
```



#### 安装与配置

- 安装

```shell
tar zxf lkp-extent-${LKP_EXTENT_VERSION}-node.tar.gz
cd lkp-extent-${LKP_EXTENT_VERSION}-node
./install.sh
```

- 配置

    配置文件为/etc/lkp/node.yaml

```yaml
lkp-master:
  monitor:
    ip: 127.0.0.1              # lkp-master的服务IP地址
    port: 8883                 # lkp-master的服务端口号
    
lkp-node:
  proxy:
    port: 28928                # 本地客户端用来接收lkp指令的服务端口号

testcase:
  path: "/var/run/lkp/cases/"  # 指定远程分发测试用例存放的目录

result:
  path: "/var/run/lkp/result/" # 指定测试结果包搞存放的目录
```



#### 运行前环境检查

- 主机能够访问lkp-master的服务IP，且lkp-master的服务端处于监听状态
- 检查本地的28928端口是否被占用。如果端口被占用，请修改配置文件，为之分配一个未被占用的端口。



#### 运行

```shell
./lkp-node
```

