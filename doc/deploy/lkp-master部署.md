### lkp-master部署

#### 部署前环境检查

- 主机需要能够访问外部网络，从公共网络拉取安装包以及保证lkp-node能够接入。



#### 构建lkp-extent

请参照[lkp-extent构建](../build/lkp-extent构建.md)



#### 下载lkp-extent-master二进制包

```shell
wget https://github.com/AntiBargu/lkp-extent/releases/download/${LKP_EXTENT_VERSION}/lkp-extent-${LKP_EXTENT_VERSION}-master.tar.gz
```

样例：

```shell
export LKP_EXTENT_VERSION=v0.2.2
wget https://github.com/AntiBargu/lkp-extent/releases/download/${LKP_EXTENT_VERSION}/lkp-extent-${LKP_EXTENT_VERSION}-master.tar.gz
```



#### 安装与配置

- 安装

```shell
tar zxf lkp-extent-${LKP_EXTENT_VERSION}-master.tar.gz
cd lkp-extent-${LKP_EXTENT_VERSION}-master
./install.sh
```

- 配置

    配置文件为/etc/lkp/cfg.yaml

```yaml
service:
  cli-daemon:
    port: 8999               # cli服务的端口号
    sock: /var/run/lkp.sock  # cli服务监听的unix socket文件

  monitor:
    port: 8883               # 监控lkp-node的服务端口号
    keepalive: 10            # 检测lkp-node空闲链接的时间间隔 (单位: 秒)
    keepalivetimeout: 5      # 当探活请求超过设置时间后，master主动关闭不活跃链接 (单位: 秒)

resource:
    testcase:
      limit: 128             # 配置显示test case列表的最大长度
    job:
      limit: 2048            # 配置显示job列表的最大长度
    task:
      limit: 65536           # 配置显示task列表的最大长度

feature:
  timestamp:
    fmt: 2006-01-02 15:04:05 # 打印时间戳的格式
```



#### 运行前环境检查

- 检查本地的8999、8883端口以及/var/run/lkp.sock是否被占用。如果端口被占用，请修改配置文件，为之分配一个未被占用的端口。



#### 运行

```shell
lkp-master
```

