# queue：任务队列

> 代码路径：src/queue/ ｜ 父页：[index](../index.md) ｜ 相关模块：[tm](./tm.md)、[sched](./sched.md)

## 职责与边界

实现 `ffrt_queue_*` API：串行队列（顺序执行）与并发队列（限并发度）。`QueueHandler` 管理队列属性与 worker；`SerialQueue/ConcurrentQueue` 是两种策略；`QueueMonitor/TrafficRecord` 做队列监控；`EventHandlerAdapterQueue` 把 FFRT 队列适配成 EventHandler 队列。

## 关键类与文件

| 文件 / 类 | 职责 | 备注 |
| --- | --- | --- |
| [src/queue/queue_api.cpp:196](../../../../src/queue/queue_api.cpp) | `ffrt_queue_create/submit/...` C API | 入口 |
| [src/queue/queue_handler.h](../../../../src/queue/queue_handler.h) / `QueueHandler` | 队列句柄：属性 + worker 管理 | |
| [src/queue/serial_queue.cpp](../../../../src/queue/serial_queue.cpp) | 串行策略 | |
| [src/queue/concurrent_queue.cpp](../../../../src/queue/concurrent_queue.cpp) | 并发策略 | |
| [src/queue/base_queue.cpp](../../../../src/queue/base_queue.cpp) / `BaseQueue` | 公共基类 | |
| [src/queue/queue_monitor.cpp](../../../../src/queue/queue_monitor.cpp) | 队列监控 | Facade 单例 |
| [src/queue/eventhandler_adapter_queue.cpp](../../../../src/queue/eventhandler_adapter_queue.cpp) | EventHandler 适配 | 系统集成 |

## 工作机制 / 主流程

1. `ffrt_queue_create` → new `QueueHandler`（[queue_api.cpp:201](../../../../src/queue/queue_api.cpp)）。
2. `ffrt_queue_submit` → [ffrt_queue_submit_base:42](../../../../src/queue/queue_api.cpp) 构造 `QueueTask`（闭包同样前置存储）。
3. `QueueTask::Ready()`（[queue_task.cpp:80](../../../../src/tm/queue_task.cpp)）→ `Scheduler::PushTask` → `ExecuteUnit::NotifyTask`，与普通任务共用调度与 Worker。
4. 串行/并发语义由队列侧的 worker 绑定与并发计数实现；串行队列同一时刻只有一个任务在执行。

## 依赖（上游）

| 被依赖方 | 类型 | 说明 | 证据 |
| --- | --- | --- | --- |
| tm | call | QueueTask 构造与 Ready | [queue_task.cpp:86](../../../../src/tm/queue_task.cpp) |
| sched | call | 最终入同一调度器 | [queue_task.cpp:86](../../../../src/tm/queue_task.cpp) |
| util | call | 事件处理适配 | [event_handler_adapter.h](../../../../src/util/event_handler_adapter.h) |

## 被谁依赖（下游）

- api：`ffrt_queue_*`（[queue_api.cpp:215](../../../../src/queue/queue_api.cpp)）。

## 待确认 / 遗留

- [ ] 串行/并发策略差异的细节：读 [queue_strategy.h](../../../../src/queue/queue_strategy.h) 与两个实现 cpp。
- [ ] `EventHandlerInteractiveQueue` 的交互场景：需结合使用方确认。

## 相关文档

- [index](../index.md) ｜ [tm](./tm.md) ｜ [sched](./sched.md)
