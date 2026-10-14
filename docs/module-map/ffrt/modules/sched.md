# sched：调度器

> 代码路径：src/sched/、include/sched/ ｜ 父页：[index](../index.md) ｜ 相关模块：[tm](./tm.md)、[eu](./eu.md)

## 职责与边界

按 QoS 维度管理任务运行队列。`Scheduler` 聚合 `QoS::MaxNum()` 个 `TaskScheduler`（每个 QoS 一个），`STaskScheduler` 是默认实现（支持 global queue + worker 本地队列 + steal）。另含帧调度（deadline/frame_interval）、负载跟踪等高级特性。

## 关键类与文件

| 文件 / 类 | 职责 | 备注 |
| --- | --- | --- |
| [include/sched/scheduler.h:24](../../../../include/sched/scheduler.h) / `Scheduler` | per-QoS 调度器数组，PushTask/PopTask 门面 | Facade 单例 |
| [include/sched/task_scheduler.h:26](../../../../include/sched/task_scheduler.h) / `TaskScheduler` | 抽象：运行队列 + UV 任务并发控制 | |
| [src/sched/stask_scheduler.h](../../../../src/sched/stask_scheduler.h) / `STaskScheduler` | 默认实现 | global + local + steal |
| [include/sched/task_runqueue.h](../../../../include/sched/task_runqueue.h) | 运行队列抽象 | |
| [src/sched/task_scheduler.cpp](../../../../src/sched/task_scheduler.cpp) | UV 任务等待/取消逻辑 | |
| [src/sched/deadline.cpp](../../../../src/sched/deadline.cpp) | 帧截止时间调度 | QoS 与帧调度协作 |
| [src/sched/load_tracking.cpp](../../../../src/sched/load_tracking.cpp) | 负载跟踪 | |

## 工作机制 / 主流程

1. `CPUEUTask::Ready()` → `Scheduler::PushTask(task, rtb=false)`：按 `task->qos_` 找到对应 `taskSchedulers[qos]`（[scheduler.h:40](../../../../include/sched/scheduler.h)）。
2. Worker 空闲时 `PopTask(qos)` 从对应队列取任务；`stealingInProgress` 标记防止偷取风暴。
3. 每个 QoS 一个全局调度锁 `g_schedMtx[qos]`（[ffrt_facade.h:44](../../../../src/util/ffrt_facade.h)），Worker 组与调度器共享，减少锁冲突。
4. UV 任务有单独的并发上限与等待队列（`uvTaskConcurrency_`、`uvTaskWaitingQueue_`，[task_scheduler.h:79](../../../../include/sched/task_scheduler.h)）。

## 依赖（上游）

| 被依赖方 | 类型 | 说明 | 证据 |
| --- | --- | --- | --- |
| tm | data | 队列元素是 TaskBase* | [scheduler.h:40](../../../../include/sched/scheduler.h) |
| util | call | SchedulerFactory 注册 | [src/util/init.cpp:66](../../../../src/util/init.cpp) |

## 被谁依赖（下游）

- tm：Ready 入队（[cpu_task.cpp:54](../../../../src/tm/cpu_task.cpp)）。
- eu：Worker 取任务（[cpu_worker.cpp:246](../../../../src/eu/cpu_worker.cpp)）。

## 待确认 / 遗留

- [ ] 帧调度（deadline/frame_interval/multi_workgroup）与 QoS 调度的交互关系：接力时细读 [src/sched/deadline.cpp](../../../../src/sched/deadline.cpp)。
- [ ] worker 本地队列与偷取的具体实现：读 [stask_scheduler.h](../../../../src/sched/stask_scheduler.h)。

## 相关文档

- [index](../index.md) ｜ [tm](./tm.md) ｜ [eu](./eu.md)
