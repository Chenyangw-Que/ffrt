# dfx：维测

> 代码路径：src/dfx/、include/dfx/ ｜ 父页：[index](../index.md) ｜ 相关模块：全部模块均调用

## 职责与边界

可观测性与故障处置：HiTrace 打点、HiSysEvent 上报、日志、dump（ffrt_dump）、watchdog（任务超时）、bbox（黑盒冻结）、异步栈、trace_record（事件记录）、perf 打点。

## 关键类与文件

| 文件 / 类 | 职责 | 备注 |
| --- | --- | --- |
| [src/dfx/trace/ffrt_trace.cpp](../../../../src/dfx/trace/ffrt_trace.cpp) | HiTrace 打点 | FFRT_OH_TRACE_ENABLE 宏 |
| [src/dfx/trace/ffrt_trace_chain.cpp](../../../../src/dfx/trace/ffrt_trace_chain.cpp) | trace chain（跨任务链） | submit 时建 span |
| [src/dfx/trace_record/ffrt_trace_record.cpp](../../../../src/dfx/trace_record/ffrt_trace_record.cpp) | 事件记录（submit/enqueue/execute/done） | |
| [src/dfx/sysevent/sysevent.cpp](../../../../src/dfx/sysevent/sysevent.cpp) | HiSysEvent 上报 | hisysevent.yaml 定义事件 |
| [src/dfx/log/ffrt_log.cpp](../../../../src/dfx/log/ffrt_log.cpp) | FFRT_LOG* 日志 | 分级可控 |
| [src/dfx/dump/dump.cpp](../../../../src/dfx/dump/dump.cpp) | ffrt_dump：队列/任务快照 | |
| [src/dfx/watchdog/watchdog_util.cpp](../../../../src/dfx/watchdog/watchdog_util.cpp) | 任务超时看门狗 | attr.timeout_ |
| [src/dfx/bbox/bbox.cpp](../../../../src/dfx/bbox/bbox.cpp) | 黑盒：检查并冻结 | FreeMem 时调用 |
| [src/dfx/async_stack/ffrt_async_stack.cpp](../../../../src/dfx/async_stack/ffrt_async_stack.cpp) | 异步调用栈 | FFRT_ASYNC_STACKTRACE 宏 |

## 工作机制 / 主流程

任务全生命周期打点：submit（`FFRT_SUBMIT_MARKER`）→ enqueue（`TaskEnqueue`）→ execute（`TaskExecute`）→ done（`TaskDone`），最终由 [tools/ffrt_trace_process](../../../../tools/ffrt_trace_process/readme.md) 工具解析 trace 还原任务流。

watchdog：任务带 `timeout_` 属性时，dm 在 onSubmit 注册（[sdependence_manager.cpp:121](../../../../src/dm/sdependence_manager.cpp)），Execute 时移除（[cpu_task.cpp:94](../../../../src/tm/cpu_task.cpp)），超时触发回调或 sysevent。

bbox：任务释放前 `BboxCheckAndFreeze`（[cpu_task.cpp:61](../../../../src/tm/cpu_task.cpp)），检测到异常冻结现场。

## 依赖（上游）

| 被依赖方 | 类型 | 说明 | 证据 |
| --- | --- | --- | --- |
| OHOS HiTrace/HiSysEvent | event | 外部维测框架 | [hisysevent.yaml](../../../../hisysevent.yaml) |

## 被谁依赖（下游）

- dm/tm/eu/sched/sync/queue：全链路打点（各模块 `FFRT_TRACE/FFRT_LOG*` 调用）。
- util：WorkerMonitor 监控 Worker 存活（[ffrt_facade.h](../../../../src/util/ffrt_facade.h)）。

## 配置与常量

- 宏：`FFRT_OH_TRACE_ENABLE`、`FFRT_TRACE_RECORD_LEVEL`、`FFRT_OH_EVENT_RECORD`、`FFRT_ASYNC_STACKTRACE`、`FFRT_BBOX_ENABLE`（BUILD.gn / ffrt.gni）。

## 待确认 / 遗留

- [ ] `ffrt_perf.h` 的 perf 打点分组机制（DM/EU 等 group）的完整清单。

## 相关文档

- [index](../index.md) ｜ [util](./util.md)
