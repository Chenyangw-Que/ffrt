# sync：同步原语

> 代码路径：src/sync/、include/sync/ ｜ 父页：[index](../index.md) ｜ 相关模块：[eu](./eu.md)、[tm](./tm.md)

## 职责与边界

提供“协程感知”的同步原语：mutex、recursive mutex、shared_mutex、condition_variable、sleep、timer。与 pthread 原语的本质区别：任务持锁失败或睡眠时不挂起线程，而是 `CoWait` 让出协程（根任务除外），Worker 线程得以执行其他任务。

## 关键类与文件

| 文件 / 类 | 职责 | 备注 |
| --- | --- | --- |
| [src/sync/mutex.cpp:354](../../../../src/sync/mutex.cpp) | ffrt_mutex C API + 锁实现 | normal/recursive |
| [src/sync/condition_variable.cpp](../../../../src/sync/condition_variable.cpp) | ffrt_cond C API | |
| [src/sync/shared_mutex.cpp](../../../../src/sync/shared_mutex.cpp) | 读写锁 | |
| [src/sync/sleep.cpp:51](../../../../src/sync/sleep.cpp) | sleep_for/sleep_until | CoWait + DelayedWorker |
| [src/sync/wait_queue.cpp](../../../../src/sync/wait_queue.cpp) | 等待队列：挂起/唤醒协程 | 各原语共用底座 |
| [src/sync/timer_manager.cpp](../../../../src/sync/timer_manager.cpp) | 定时器管理 | Facade 单例 |
| [src/sync/delayed_worker.cpp](../../../../src/sync/delayed_worker.cpp) | 延迟任务线程 | sleep 的时序驱动 |
| [src/sync/thread.cpp](../../../../src/sync/thread.cpp) | ffrt thread（inner_api） | |

## 工作机制 / 主流程

以 `sleep_for` 为例（[sleep.cpp:36](../../../../src/sync/sleep.cpp)）：

1. 把到期时间挂到 `DelayedWorker`/`TimerManager`；
2. `CoWait` 注册“到期回调唤醒本协程”后让出；
3. Worker 转去执行其他任务；
4. 定时器到期 → `WaitQueue` 唤醒协程 → 该任务重新 Ready。

mutex 的 `lock_wait/unlock_wake`（[mutex.cpp:243](../../../../src/sync/mutex.cpp)）同理：锁不可得时 `CoWait`，释放时从等待队列唤醒。

## 依赖（上游）

| 被依赖方 | 类型 | 说明 | 证据 |
| --- | --- | --- | --- |
| eu | call | CoWait/协程切换 | [sleep.cpp:51](../../../../src/sync/sleep.cpp) |
| tm | data | 等待队列挂的是 CoTask | [wait_queue.h](../../../../src/sync/wait_queue.h) |

## 被谁依赖（下游）

- api：mutex/cv/sleep/timer C/C++ API（`interfaces/kits/c/*.h`）。
- dm/core：criticalMutex_ 等内部锁（[entity.h:29](../../../../src/core/entity.h)）。

## 待确认 / 遗留

- [ ] `record_mutex/record_condition_variable`（record 系列）：疑似 trace 记录专用变体，待确认调用方。

## 相关文档

- [index](../index.md) ｜ [eu](./eu.md) ｜ [tm](./tm.md) ｜ [api](./api.md)
