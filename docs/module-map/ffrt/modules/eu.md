# eu：执行单元

> 代码路径：src/eu/、include/eu/ ｜ 父页：[index](../index.md) ｜ 相关模块：[sched](./sched.md)、[tm](./tm.md)、[sync](./sync.md)

## 职责与边界

管理真正干活的线程与协程：`ExecuteUnit`（实现 `SExecuteUnit`）持有每个 QoS 的 `CPUWorkerGroup`（Worker 线程池 + RTG 线程组）；`CPUWorker::WorkerLooper` 是线程主循环；`CoRoutine` 实现协程切换（`CoWait/CoStart`）；poller 系（IOPoller/LoopPoller）处理 fd/loop 事件；`qos_interface/rtg_ioctl` 对接 OHOS QoS 与 RTG 内核接口。

## 关键类与文件

| 文件 / 类 | 职责 | 备注 |
| --- | --- | --- |
| [include/eu/execute_unit.h:97](../../../../include/eu/execute_unit.h) / `ExecuteUnit` | Worker 池管理接口，Facade 单例 | NotifyTask 唤醒 |
| [src/eu/sexecute_unit.h](../../../../src/eu/sexecute_unit.h) / `SExecuteUnit` | 实现：创建/缩容 Worker | |
| [src/eu/cpu_worker.cpp:239](../../../../src/eu/cpu_worker.cpp) / `WorkerLooper` | Worker 主循环 | PopTask→Execute |
| [src/eu/cpu_worker.cpp:184](../../../../src/eu/cpu_worker.cpp) / `ExecuteTask` | 执行任务 | 配合协程 |
| [include/eu/co_routine.h](../../../../include/eu/co_routine.h) / `CoRoutine` | 协程对象：CoWait/CoStart | co2_context.c 是底座 |
| [src/eu/io_poller.cpp](../../../../src/eu/io_poller.cpp) / `IOPoller` | epoll 封装 | sync_io/loop 用 |
| [src/eu/loop_poller.cpp](../../../../src/eu/loop_poller.cpp) | loop 事件 | |
| [src/eu/qos_interface.cpp](../../../../src/eu/qos_interface.cpp) | OHOS QoS 系统调用封装 | USE_OHOS_QOS 宏 |
| [src/eu/rtg_ioctl.cpp](../../../../src/eu/rtg_ioctl.cpp) | RTG（线程组）ioctl | QOS_WORKER_FRAME_RTG 宏 |

## 工作机制 / 主流程

Worker 主循环（[cpu_worker.cpp:239](../../../../src/eu/cpu_worker.cpp)）：取任务 → `ExecuteTask` → `task->Execute()` → 用户函数。任务内再 submit 子任务即为嵌套；任务内阻塞（mutex/sleep/wait）时：

- 协程模式：`CoWait` 让出当前协程，Worker 去执行其他任务；
- 线程模式（根任务/关闭协程）：挂起整个线程。

Worker 池伸缩：`CPUWorkerGroup` 维护 `executingNum/sleepingNum/deepSleepingWorkerNum`，支持 worker share（跨 QoS 共享线程）、fast wake（futex 直达）、escape 上报等策略（[execute_unit.h:34](../../../../include/eu/execute_unit.h)）。

协程底座：`co2_context.c` 提供上下文切换汇编；`CoRoutineFactory` 池化协程对象；`CoStackAttr`（Facade 单例）管理栈。

## 依赖（上游）

| 被依赖方 | 类型 | 说明 | 证据 |
| --- | --- | --- | --- |
| sched | call | Worker 从调度器取任务 | [cpu_worker.cpp:246](../../../../src/eu/cpu_worker.cpp) |
| tm | call | 执行任务对象 | [cpu_worker.cpp:184](../../../../src/eu/cpu_worker.cpp) |
| dfx | call | trace/monitor | cpu_worker.cpp 各打点 |
| kernel | call | QoS/RTG 系统调用 | [qos_interface.cpp](../../../../src/eu/qos_interface.cpp)、[rtg_ioctl.cpp](../../../../src/eu/rtg_ioctl.cpp) |

## 被谁依赖（下游）

- tm：Ready 后 NotifyTask 唤醒（[cpu_task.cpp:57](../../../../src/tm/cpu_task.cpp)）。
- sync/dm：CoWait 协程阻塞（[sleep.cpp:51](../../../../src/sync/sleep.cpp)、[sdependence_manager.cpp:190](../../../../src/dm/sdependence_manager.cpp)）。
- util：ffrt_init 注册实现（[init.cpp:71](../../../../src/util/init.cpp)）。

## 配置与常量

- 宏 `USE_OHOS_QOS`：走系统 QoS；否则用仓库内 staging QoS（BUILD.gn 定义）。
- 宏 `QOS_WORKER_FRAME_RTG`：启用 RTG 线程组。
- `FFRT_CLOSE_COROUTINE_MODE`（TSAN 时）：关闭协程，纯线程模式。

## 待确认 / 遗留

- [ ] worker share（跨 QoS 共享 Worker）的完整策略：读 [osattr_manager.cpp](../../../../src/eu/osattr_manager.cpp)。
- [ ] `wgcm.h` 的用途：文件名含义待查。

## 相关文档

- [index](../index.md) ｜ [sched](./sched.md) ｜ [tm](./tm.md) ｜ [sync](./sync.md)
