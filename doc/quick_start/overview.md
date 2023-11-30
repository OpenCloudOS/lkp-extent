### 概述

当前版本的lkp-master负责管理以下4类资源对象：

| 资源 |                            描 述                             |
| :--: | :----------------------------------------------------------: |
| node | 用于描述接入的lkp-node信息。以便于lkp-master能够向lkp-node下发指令。 |
| case | 用于描述一个测试用例文件及其版本信息。类似docker管理image，lkp-master以name:tag的方式标识一个特定版本的测试用例文件。 |
| job  | 对于每一个集群测试任务，用job进行描述。job描述了测试用例、运行测试的lkp-node列表等信息。一个job对象包含一个或多个task对象。 |
| task | 用于描述一次具体运行测试进程的信息。包含测试用例、运行测试的环境等信息。 |
