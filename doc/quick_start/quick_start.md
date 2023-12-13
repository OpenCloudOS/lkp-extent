### 使用lkp-extent执行一次集群测试任务

#### 1. 查看lkp-extent系统版本信息

当lkp-extent部署完毕后，我们可以通过查看系统版本的方式确认我们的安装是成功的。以下命令均可以查看lkp-extent的系统版本：

```shell
lkp-ctl -v
lkp-ctl --version
```



#### 2. 查看lkp-node的接入情况

我们可以通过以下命令查看lkp-node的接入情况：

```shell
lkp-ctl ls
lkp-ctl list
lkp-ctl nodes
lkp-ctl node ls
lkp-ctl node list
```

执行结果：

```shell
NODE ID           IP             PORT   KERNEL VERSION                DISTRIBUTION                            ARCH                                    MEM
9392de519837      127.0.0.1      47662  5.4.119-20.0009.21.spr        OpenCloudOS 8.8                         Intel(R) Xeon(R) CPU E5-26xx v4         1.68GB
5a53a2c45971      43.163.234.142 42710  5.4.119-20.0009.21.spr        OpenCloudOS 8.8                         Intel(R) Xeon(R) CPU E5-26xx v4         1.68GB
```



#### 3. 向lkp-master添加测试用例文件

在执行测试任务前，测试人员需要将测试用例文件添加到lkp-master中，由lkp-master负责维护测试用例文件的版本。其中，测试用例文件用yaml格式进行描述，是[lkp-tests](https://github.com/intel/lkp-tests)能够识别的测试用例文件类型。

我们可以通过下述命令向lkp-master添加测试用例文件：

```shell
lkp-ctl add
lkp-ctl case add
```

例如：

```shell
lkp-ctl case add /lkp-tests/jobs/stress-ng-class-cpu-cache
```



#### 4. 查看测试用例文件列表

当我们执行完添加测试用例文件后，我们可以通过下述命令查看测试用例文件列表，以确保测试用例文件正确添加。

```shell
lkp-ctl cases
lkp-ctl case ls
lkp-ctl case list
```



#### 5. 将测试用例文件推送至待测试服务器

添加测试用例文件后，执行测试任务前，我们需要将测试用例推送至待测试服务器上。

我们可以通过下述命令将测试用例文件推送至lkp-node：

```shell
lkp-ctl push
lkp-ctl case push
```

例如：

```shell
# 将测试用例文件stress-ng-class-cpu-cache推送至所有接入的待测试服务器上
lkp-ctl push -a stress-ng-class-cpu-cache
```



#### 6. 执行一次集群测试任务

当我们将测试用例文件推送至lkp-node后，就可以执行测试任务了。

我们可以通过下述命令执行测试任务：

```shell
lkp-ctl run
lkp-ctl job run
```

例如：

```shell
# 所有的待测试服务器都执行stress-ng-class-cpu-cache测试，并且在容器环境中也执行1次该测试。
lkp-ctl run -a -c 1 stress-ng-class-cpu-cache
```



#### 7. 查看测试作业(job)列表

每当我们运行一次run子命令，lkp-master就会创建一个job对象。我们可以通过以下命令查看job列表，以确保我们成功运行一次集群测试。

```shell
lkp-ctl jobs
lkp-ctl job ls
lkp-ctl job list
```



#### 8. 查看测试任务(task)列表

我们也可以查看指定测试job的测试task列表，task列表记录了每个测试task的运行状态。

```shell
lkp-ctl tasks
lkp-ctl task ls
lkp-ctl task list
```



#### 9. 获取测试结果

当测试运行完毕后，测试task的状态会由**RUNNING**转换至**FINISHED**，我们可以通过下述命令获取测试结果所在的文件。

```shell
lkp-ctl result
```

测试结果是一个.tar.gz的压缩归档文件。测试人员可以这样在一个目录中查看我们的测试结果：

```shell
mkdir ${result_dir}
cd ${result_dir}
tar zxf $(lkp-ctl result ${task_id})
```

