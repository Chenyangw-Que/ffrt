# dm：依赖管理器

> 代码路径：src/dm/ ｜ 父页：[index](../index.md) ｜ 相关模块：[core](./core.md)、[tm](./tm.md)、[eu](./eu.md)

## 职责与边界

`DependenceManager` 是 submit/wait/done 的编排接口；`SDependenceManager` 是其唯一实现。它把“任务函数 + 依赖描述”变成任务对象并挂入依赖图；wait 时让当前任务阻塞（协程让出或线程挂起）；任务完成后驱动依赖图解阻塞。它不负责队列和运行队列，那是 [queue](./queue.md) / [sched](./sched.md) 的事。

## 关键类与文件

| 文件 / 类 | 职责 | 备注 |
| --- | --- | --- |
| [include/dm/dependence_manager.h:54](../../../../include/dm/dependence_manager.h) / `DependenceManager` | 抽象接口：onSubmit/onWait/onTaskDone/onSkip/DecTaskRef | Facade 单例之一 |
| [src/dm/sdependence_manager.cpp:68](../../../../src/dm/sdependence_manager.cpp) / `SDependenceManager::onSubmit` | 提交主流程 | 建任务 + 建图 |
| [src/dm/sdependence_manager.cpp:171](../../../../src/dm/sdependence_manager.cpp) / `onWait()` | 等全部子任务 | join 语义 |
| [src/dm/sdependence_manager.cpp:211](../../../../src/dm/sdependence_manager.cpp) / `onWait(deps)` | 等指定数据 | wait_deps 语义 |
| [src/dm/sdependence_manager.cpp:289](../../../../src/dm/sdependence_manager.cpp) / `MapSignature2Deps` | 签名 → VersionCtx 映射 | 依赖 [core](./core.md) |

## 工作机制 / 主流程

`onSubmit`（[sdependence_manager.cpp:68](../../../../src/dm/sdependence_manager.cpp)）：

1. 校验 outs 中不能出现 handle（[CheckOutsHandle:27](../../../../include/dm/dependence_manager.h)）。
2. 取当前执行上下文的父任务；根上下文用 thread_local 的 `RootTaskCtxWrapper`。
3. **placement new**：用户闭包 `f` 的地址回退 `OFFSETOF(SCPUEUTask, func_storage)` 得到任务对象地址——闭包和任务是同一块内存（[sdependence_manager.cpp:96](../../../../src/dm/sdependence_manager.cpp)）。
4. 设置 QoS（支持 `qos_inherit` 继承父任务）、trace、watchdog 超时。
5. `RemoveRepeatedDeps` 对 ins/outs 去重；handle 依赖转成 `in_handles_`。
6. `MapSignature2Deps` 把签名挂到 VersionCtx：输入 `AddConsumer`，输出 `AddProducer`。
7. 若 `submitDep != 0`（依赖未满足）→ 留在图上等待；否则 `task->Ready()`。

`onWait`：`Block()` 判断协程还是线程模式；协程走 `CoWait`（让出协程），根任务/线程模式走 `waitCond_.wait`。等待目标分别是 `childRefCnt`（join）或 `dataRefCnt.waitDep`（数据）。

`onTaskDone`：在 `criticalMutex_` 保护下解依赖图，通知被唤醒的消费者。

## 对外接口

- 被 [api](./api.md) 的 submit/wait 系列调用。

## 依赖（上游）

| 被依赖方 | 类型 | 说明 | 证据 |
| --- | --- | --- | --- |
| core | call | 签名映射与解依赖 | [sdependence_manager.cpp:137](../../../../src/dm/sdependence_manager.cpp) |
| tm | call | 构造 SCPUEUTask、计数管理 | [sdependence_manager.cpp:96](../../../../src/dm/sdependence_manager.cpp) |
| eu | call | CoWait 协程阻塞 | [sdependence_manager.cpp:190](../../../../src/dm/sdependence_manager.cpp) |
| dfx | call | trace/watchdog/sysevent | [sdependence_manager.cpp:71](../../../../src/dm/sdependence_manager.cpp) |

## 被谁依赖（下游）

- api：`submit_impl`、`ffrt_wait`（[src/core/task.cpp:76](../../../../src/core/task.cpp)）。
- util：`ffrt_init` 注册 `SDependenceManager::Instance`（[src/util/init.cpp:70](../../../../src/util/init.cpp)）。

## 待确认 / 遗留

- [ ] `onSkip` 的取消语义与 handle 引用计数交互：读 [dependence_manager.h:67](../../../../include/dm/dependence_manager.h) 与调用方。

## 相关文档

- [index](../index.md) ｜ [core](./core.md) ｜ [tm](./tm.md) ｜ [eu](./eu.md)
