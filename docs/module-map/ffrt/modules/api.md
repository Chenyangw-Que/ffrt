# api：FFRT 对外接口层

> 代码路径：interfaces/、src/core/task.cpp ｜ 父页：[index](../index.md) ｜ 相关模块：[dm](./dm.md)、[sync](./sync.md)、[queue](./queue.md)

## 职责与边界

提供用户可见的 C/C++ API：任务提交与等待、任务属性（QoS/延时/超时）、同步原语、队列、sleep/timer/loop/poller。API 声明在 `interfaces/kits`（公开）与 `interfaces/inner_api`（系统内部），实现分散在 `src/core`、`src/sync`、`src/queue` 等目录的 `*_api.cpp` / 对应 cpp。

## 关键类与文件

| 文件 / 接口 | 职责 | 备注 |
| --- | --- | --- |
| [interfaces/kits/c/task.h](../../../../interfaces/kits/c/task.h) | `ffrt_task_submit/submit_h/wait/wait_deps` 等 C API | 核心入口 |
| [interfaces/kits/cpp/task.h](../../../../interfaces/kits/cpp/task.h) | C++ 模板封装 | submit/wait 的类型安全版本 |
| [src/core/task.cpp:76](../../../../src/core/task.cpp) | `submit_impl` → `DependenceManager::onSubmit` | 提交链路枢纽 |
| [src/core/task_attr_private.h](../../../../include/core/task_attr_private.h) | `task_attr_private`：QoS/延时/超时/名字 | attr 的运行时形态 |
| [src/queue/queue_api.cpp:196](../../../../src/queue/queue_api.cpp) | `ffrt_queue_create/submit` | 队列入口 |
| [src/sync/mutex.cpp:354](../../../../src/sync/mutex.cpp) | `ffrt_mutex_lock` 等 | 同步入口 |

## 工作机制 / 主流程

1. 用户调 `ffrt_task_submit(func, in_deps, out_deps, attr)`。
2. [submit_impl:76](../../../../src/core/task.cpp) 直接转调 `FFRTFacade::GetDependenceManager().onSubmit(...)`。
3. attr 中 `delay_` 非零时走延时路径：[CreateDelayDeps:114](../../../../src/core/task.cpp) 包一层 `sleep_for` 的任务。
4. `ffrt_wait` → `onWait()`；`ffrt_wait_deps` → `onWait(deps)`（见 [dm.md](./dm.md)）。

## 对外接口

- C：task / queue / mutex / condvar / sleep / timer / loop / fiber（`interfaces/kits/c/*.h`）。
- C++：task / queue / mutex 等模板封装 + job_partner 等 pattern（`interfaces/kits/cpp/`）。
- inner_api：deadline、executor_task、cpu_boost、dump、ipc、thread（系统内部使用）。

## 依赖（上游）

| 被依赖方 | 类型 | 说明 | 证据 |
| --- | --- | --- | --- |
| dm | call | 提交/等待全部进入依赖管理器 | [src/core/task.cpp:76](../../../../src/core/task.cpp) |
| queue | call | 队列 API 直通 QueueHandler | [src/queue/queue_api.cpp:215](../../../../src/queue/queue_api.cpp) |
| sync | call | mutex/cv/sleep API | [src/sync/mutex.cpp:354](../../../../src/sync/mutex.cpp) |

## 被谁依赖（下游）

- 最终用户与上层系统服务：通过 `libffrt` 动态库使用（`libffrt.map` 定义导出符号）。

## 待确认 / 遗留

- [ ] inner_api 中 `fiber.h` 与协程暴露边界：需要结合使用方仓库确认。

## 相关文档

- [index](../index.md) ｜ [dm](./dm.md) ｜ [tm](./tm.md) ｜ [queue](./queue.md)
