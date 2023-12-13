### lkp-ctl部署

#### 部署前环境检查

- 主机需要能够访问外部网络，从公共网络拉取安装包。



#### 构建lkp-extent

请参照[lkp-extent构建](../build/lkp-extent构建.md)



#### 下载lkp-extent-ctl二进制包

```shell
wget https://github.com/AntiBargu/lkp-extent/releases/download/${LKP_EXTENT_VERSION}/lkp-extent-${LKP_EXTENT_VERSION}-ctl.tar.gz
```

样例：

```shell
export LKP_EXTENT_VERSION=v0.2.2
wget https://github.com/AntiBargu/lkp-extent/releases/download/${LKP_EXTENT_VERSION}/lkp-extent-${LKP_EXTENT_VERSION}-ctl.tar.gz
```



#### 安装

```shell
tar zxf lkp-extent-${LKP_EXTENT_VERSION}-ctl.tar.gz
cd lkp-extent-${LKP_EXTENT_VERSION}-ctl
./install.sh
```



#### 运行前环境检查

- 运行前需保证lkp-master服务已经启动。



#### 运行

```shell
lkp-ctl
```

