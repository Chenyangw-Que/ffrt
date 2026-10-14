# tm：任务对象族与工厂

> 代码路径：src/tm/、include/tm/ ｜ 父页：[index](../index.md) ｜ 相关模块：[dm](./dm.md)、[sched](./sched.md)、[eu](./eu.md)

## 职责与边界

定义任务的运行时对象与生命周期状态机。`TaskBase` 声明 Prepare/Ready/Block/Wake/Execute/Finish/Cancel/FreeMem 虚接口；`CoTask` 挂接协程；`CPUEUTask`（实现类 `SCPUEUTask`）是普通任务；`QueueTask` 继承 CoTask（服务队列）；`IOTask`/`UVTask` 直接继承 TaskBase（无协程绑定，分别服务 fd IO 与 libuv 事件）。`TaskFactory` + `SimpleAllocator`（slab）负责对象分配。

## 关键类与文件

| 文件 / 类 | 职责 | 备注 |
| --- | --- | --- |
| [include/tm/task_base.h:66](../../../../include/tm/task_base.h) / `TaskBase` | 状态机接口 + gid/qos/引用计数 | 全部任务的基类 |
| [include/tm/task_base.h:140](../../../../include/tm/task_base.h) / `CoTask` | 增加协程绑定、等待队列条目 | `func_storage` 内联闭包存储 |
| [include/tm/cpu_task.h:65](../../../../include/tm/cpu_task.h) / `CPUEUTask` | 普通任务：父子关系/超时/依赖计数 | |
| [src/tm/scpu_task.h](../../../../src/tm/scpu_task.h) / `SCPUEUTask` | CPUEUTask 的最终实现 | dataRefCnt/childRefCnt |
| [src/tm/cpu_task.cpp:49](../../../../src/tm/cpu_task.cpp) / `Ready` | 入队 + 唤醒 Worker | 主链路 |
| [src/tm/cpu_task.cpp:85](../../../../src/tm/cpu_task.cpp) / `Execute` | CAS 防重入后执行用户函数 | `f->exec(f)` |
| [src/tm/queue_task.cpp:80](../../../../src/tm/queue_task.cpp) / `QueueTask::Ready` | 队列任务入队 | 配合队列 worker |
| [src/tm/task_factory.cpp](../../../../src/tm/task_factory.cpp) / `TaskFactory<T>` | slab 分配的工厂模板 | `Alloc/Free` |

## 工作机制 / 主流程

任务与协程的生命周期（[task_base.h:162](../../../../include/tm/task_base.h) 注释）：

```text
|task pending|task ready|task executing|task done|task release|
                         |******coroutine******|
```

- `Block()`：非根任务且协程模式 → `BLOCK_COROUTINE`；否则 `BLOCK_THREAD`（[cpu_task.h:106](../../../../include/tm/cpu_task.h)）。
- `Execute()`：`skipped` 字段 CAS（SUBMITTED→EXECUTED）保证只执行一次；协程模式下 onTaskDone 延后到协程尾部，避免任务内存被复用导致的崩溃（[cpu_task.cpp:110](../../../../src/tm/cpu_task.cpp) 注释）。
- `FreeMem()`：回收 slab、清理 poller 缓存、task-local、异步栈。
- 引用计数：`rc`（删除引用）+ `childRefCnt`（父等子）+ `dataRefCnt`（数据依赖）+ `submitDep`。

## 对外接口

- 供 dm/eu/queue 内部使用，不直接暴露给用户。

## 依赖（上游）

| 被依赖方 | 类型 | 说明 | 证据 |
| --- | --- | --- | --- |
| sched | call | Ready → PushTask | [cpu_task.cpp:54](../../../../src/tm/cpu_task.cpp) |
| eu | call | Ready → NotifyTask；Execute 挂协程 | [cpu_task.cpp:57](../../../../src/tm/cpu_task.cpp) |
| dfx | call | 状态打点/watchdog | [cpu_task.cpp:52](../../../../src/tm/cpu_task.cpp) |
| util | call | slab 分配 | [cpu_task.cpp:61](../../../../src/tm/cpu_task.cpp) |

## 被谁依赖（下游）

- dm：构造任务、维护计数（[sdependence_manager.cpp:68](../../../../src/dm/sdependence_manager.cpp)）。
- eu：Worker 执行任务（[cpu_worker.cpp:184](../../../../src/eu/cpu_worker.cpp)）。
- queue：QueueTask 分发（[queue_task.cpp:86](../../../../src/tm/queue_task.cpp)）。

## 待确认 / 遗留

- [x] `UVTask` 与 libuv 事件回流：`ffrt_executor_task_register_func` 注册类型回调（FuncManager）→ `ffrt_executor_task_submit` → OnSubmitUV → Ready（并发超限进等待队列）→ Execute 批量收割等待队列（`src/tm/uv_task.cpp`）。

## 相关文档

- [index](../index.md) ｜ [dm](./dm.md) ｜ [sched](./sched.md) ｜ [eu](./eu.md)
