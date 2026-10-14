# core：数据依赖图与任务核心

> 代码路径：src/core/ ｜ 父页：[index](../index.md) ｜ 相关模块：[dm](./dm.md)、[tm](./tm.md)

## 职责与边界

实现“数据依赖”的核心数据结构：`Entity` 把任意数据地址（signature）映射到 `VersionCtx` 版本链；`VersionCtx` 记录每个数据版本的 producer/consumer，任务完成时负责解依赖并唤醒消费者。同时 `src/core/task.cpp` 承载任务 C API（见 [api.md](./api.md)），`task_io/loop_api/poller_api/timer_api` 提供 IO 事件与定时器入口。

## 关键类与文件

| 文件 / 类 | 职责 | 备注 |
| --- | --- | --- |
| [src/core/entity.h:14](../../../../src/core/entity.h) / `Entity` | signature→VersionCtx 全局映射 + 版本回收 | `vaMap`、`versionTrashcan` |
| [src/core/entity.cpp:21](../../../../src/core/entity.cpp) / `VA2Ctx` | 取（或创建）数据地址对应的版本上下文 | dm 在 submit 时调用 |
| [src/core/version_ctx.h:21](../../../../src/core/version_ctx.h) / `VersionCtx` | 一个数据版本的依赖节点 | 双向链成版本链 |
| [src/core/version_ctx.cpp:71](../../../../src/core/version_ctx.cpp) / `onProduced` | 生产完成 → 通知消费者 | 解依赖关键路径 |
| [src/core/version_ctx.cpp:92](../../../../src/core/version_ctx.cpp) / `onConsumed` | 消费完成 → 通知下一版本生产者 | WAR 依赖 |
| [src/core/task_io.cpp](../../../../src/core/task_io.cpp) | fd 事件任务辅助 | 配合 [eu 的 IOPoller](./eu.md) |
| [src/core/loop_api.cpp](../../../../src/core/loop_api.cpp) | loop C API | 事件循环 |
| [src/core/timer_api.cpp](../../../../src/core/timer_api.cpp) | timer C API | 定时器入口 |

## 工作机制 / 主流程

依赖图的三种边（对应 `ffrt_dependence_type_t`）：

1. `ffrt_dependence_data`：用户数据地址做签名，RAW/WAR 依赖。
2. `ffrt_dependence_task`：任务 handle 依赖（仅可作输入）。
3. handle 作为输出：`onSubmit` 会把 handle 追加到 outs 签名末尾（[sdependence_manager.cpp:126](../../../../src/dm/sdependence_manager.cpp)）。

版本链结构（[version_ctx.h:14](../../../../src/core/version_ctx.h) 注释）：非嵌套时 `v1<->v2<->v3<->data`；嵌套任务在父版本下创建子版本，完成后 `MergeChildVersion` 合并。

解依赖流程：任务 `Finish` → `onTaskDone` → `onProduced/onConsumed` → `DecDepRef` 消费者计数减一 → 归零则消费者 `Ready()` 入队。

## 依赖（上游）

| 被依赖方 | 类型 | 说明 | 证据 |
| --- | --- | --- | --- |
| tm | data | VersionCtx 持有 SCPUEUTask* 作为 producer/consumer | [version_ctx.h:38](../../../../src/core/version_ctx.h) |
| sync | call | 依赖 fast_mutex 等原语 | [entity.h:10](../../../../src/core/entity.h) |

## 被谁依赖（下游）

- dm：submit 建图、done 解图（[sdependence_manager.cpp:137](../../../../src/dm/sdependence_manager.cpp)）。
- api：task/loop/poller/timer C API 实现（[src/core/task.cpp](../../../../src/core/task.cpp)）。

## 待确认 / 遗留

- [ ] 嵌套场景 `CreateChildVersion/MergeChildVersion` 的完整状态机：建议接力读 [version_ctx.cpp](../../../../src/core/version_ctx.cpp) 全文。

## 相关文档

- [index](../index.md) ｜ [dm](./dm.md) ｜ [tm](./tm.md) ｜ [eu](./eu.md)
