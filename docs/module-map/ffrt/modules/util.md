# util：基础设施与装配

> 代码路径：src/util/、src/ipc/、src/internal_inc/ ｜ 父页：[index](../index.md) ｜ 相关模块：全部模块

## 职责与边界

运行时的“装配车间”和公共底座：`FFRTFacade` 聚合全部单例（懒加载 + 全局指针快路径）；`ffrt_init` 在库加载时注册各接口的实现类；slab/spmc_queue/IntrusiveList 等提供无锁与内存基础设施；graph_check 校验用户依赖图；white_list/capability 管理权限。

## 关键类与文件

| 文件 / 类 | 职责 | 备注 |
| --- | --- | --- |
| [src/util/ffrt_facade.h:77](../../../../src/util/ffrt_facade.h) / `FFRTFacade` | 单例门面：Scheduler/DM/EU/Poller/Timer/Monitor 等 | `SINGLETON_LIST` 宏 |
| [src/util/init.cpp:52](../../../../src/util/init.cpp) / `ffrt_init` | constructor 属性，库加载即执行 | 装配入口 |
| [src/util/init.cpp:45](../../../../src/util/init.cpp) / `RegistCommonTaskFactory` | 注册 Queue/IO/UV 任务工厂 | |
| [src/util/slab.h](../../../../include/util/slab.h) / slab 分配器 | 任务/协程对象池 | TaskFactory 底座 |
| [src/util/graph_check.cpp](../../../../src/util/graph_check.cpp) | 依赖图合法性检查 | 循环依赖检测 |
| [src/util/worker_monitor.cpp](../../../../src/util/worker_monitor.cpp) | Worker 存活监控 | |
| [src/util/white_list.cpp](../../../../src/util/white_list.cpp) | 白名单 | |
| [src/util/spmc_queue.cpp](../../../../src/util/spmc_queue.cpp) | 单生产者多消费者队列 | |
| [src/internal_inc/config.h](../../../../src/internal_inc/config.h) | 内部编译配置 | |

## 工作机制 / 主流程

`ffrt_init`（[init.cpp:52](../../../../src/util/init.cpp)）装配顺序：

1. `ExecuteCtx::CtxEnvCreate()` / `DelayedWorker::ThreadEnvCreate()` / `CoEnvCreate()`：三类线程环境；
2. `RegistCommonTaskFactory()`：Queue/IO/UV 任务工厂；
3. `RegisterTaskFactoryCallbacks<CPUEUTask, SCPUEUTask>()`：普通任务工厂；
4. `SchedulerFactory::RegistCb(new STaskScheduler)`：调度器实现；
5. `DependenceManager::RegistInsCb(SDependenceManager::Instance)`：依赖管理实现；
6. `ExecuteUnit::RegistInsCb(SExecuteUnit::Instance)`：执行单元实现；
7. `SetFuncQosMap/SetFuncQosMax`：QoS 映射。

`FFRTFacade::Get*` 快路径：全局指针 `gScheduler` 等非空直接返回，首次为空才走 `LazyInstance` 懒加载（[ffrt_facade.h:87](../../../../src/util/ffrt_facade.h)）。

## 依赖（上游）

| 被依赖方 | 类型 | 说明 | 证据 |
| --- | --- | --- | --- |
| dm/tm/sched/eu/sync/queue/dfx | call | 注册实现或提供底座 | [init.cpp:57-72:57](../../../../src/util/init.cpp) |

## 被谁依赖（下游）

- 所有模块通过 `FFRTFacade::Get*()` 获取彼此（解耦 + 快路径）。

## 待确认 / 遗留

- [ ] [src/ipc/ipc.cpp](../../../../src/ipc/ipc.cpp) 单文件模块的用途：疑似配合 inner_api 的 ffrt_ipc.h，待查证调用方。

## 相关文档

- [index](../index.md) ｜ [dm](./dm.md) ｜ [tm](./tm.md) ｜ [sched](./sched.md) ｜ [eu](./eu.md)
