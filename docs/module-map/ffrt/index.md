# FFRT 模块结构地图

> 探索范围：仓库根（重点 `src/`、`include/`、`interfaces/`）
> 生成日期：2026-10-04 ｜ 状态：可接力 ｜ 语言：中文

## 一句话定位

FFRT（Function Flow Runtime）是 OpenHarmony 的用户态并发编程框架：开发者只提交“任务 + 数据依赖”，运行时负责依赖解析、按 QoS 调度、协程化执行和线程池管理，从而替代裸线程编程模型。

## 目录骨架（已精简）

```text
ffrt/
├── interfaces/          # 对外 API 头文件
│   ├── kits/            #   公开 C/C++ API（task/queue/mutex/sleep/...）
│   └── inner_api/       #   内部系统 API（deadline/ipc/dump/cpu_boost/...）
├── include/             # 对内头文件（按 eu/sched/tm/dm/sync/util/dfx 分层）
├── src/
│   ├── core/            # 任务 C API 实现 + 数据依赖图（Entity/VersionCtx）
│   ├── dm/              # 依赖管理器：submit/wait/done 的编排中枢
│   ├── tm/              # 任务对象族：CPUEUTask/QueueTask/IOTask + 工厂
│   ├── sched/           # per-QoS 任务调度器与运行队列
│   ├── eu/              # 执行单元：Worker 线程池、协程、poller、RTG/QoS
│   ├── sync/            # 协程友好的同步原语（mutex/cv/sleep/timer）
│   ├── queue/           # 串行/并发队列实现
│   ├── dfx/             # 维测：trace/log/dump/sysevent/watchdog/bbox
│   ├── util/            # Facade 单例、初始化、内存分配、graph_check
│   ├── ipc/             # IPC 辅助
│   └── internal_inc/    # 内部配置 config.h
├── test/  benchmarks/  examples/  # 测试、性能、示例
└── BUILD.gn / CMakeLists.txt      # OHOS GN 构建 + 通用 CMake 构建
```

## 模块清单

| 模块 | 代码路径 | 一句话职责 | 文档 |
| --- | --- | --- | --- |
| api | interfaces/ + src/core/task.cpp 等 | 用户可见的 C/C++ API 层，submit/wait/attr 的入口 | [modules/api.md](modules/api.md) |
| core | src/core/ | 数据依赖图（Entity→VersionCtx）与任务生命周期辅助 | [modules/core.md](modules/core.md) |
| dm | src/dm/ | 依赖管理器：submit 建图、wait 阻塞、done 传递 | [modules/dm.md](modules/dm.md) |
| tm | src/tm/ + include/tm/ | 任务对象族与工厂：CPU/Queue/IO/UV 任务 | [modules/tm.md](modules/tm.md) |
| sched | src/sched/ + include/sched/ | per-QoS 调度器与运行队列、帧调度 | [modules/sched.md](modules/sched.md) |
| eu | src/eu/ + include/eu/ | 执行单元：Worker 池、协程切换、poller、QoS/RTG | [modules/eu.md](modules/eu.md) |
| sync | src/sync/ + include/sync/ | 协程感知的 mutex/cv/sleep/timer 原语 | [modules/sync.md](modules/sync.md) |
| queue | src/queue/ | 串行/并发队列与 EventHandler 适配 | [modules/queue.md](modules/queue.md) |
| dfx | src/dfx/ + include/dfx/ | trace/log/dump/sysevent/watchdog/bbox 维测 | [modules/dfx.md](modules/dfx.md) |
| util | src/util/ + src/ipc/ | Facade 单例聚合、初始化装配、内存与工具 | [modules/util.md](modules/util.md) |

## 关系总览

[打开关系示意图](diagram.md)（含边表与图例；由 relationships.json 生成）。

文字版关键依赖（避免看图时误解）：

- api -> dm：`ffrt_task_submit` 经 `submit_impl` 进入 `DependenceManager::onSubmit`（[证据:76](../../../src/core/task.cpp)）。
- dm -> core：submit 时把 in/out 签名映射成 `VersionCtx` 依赖节点（[证据:137](../../../src/dm/sdependence_manager.cpp)）。
- dm -> tm：onSubmit 中以 placement new 构造 `SCPUEUTask`（[证据:68](../../../src/dm/sdependence_manager.cpp)）。
- tm -> sched：`CPUEUTask::Ready()` 把任务压入 per-QoS 调度器（[证据:54](../../../src/tm/cpu_task.cpp)）。
- tm -> eu：Ready 后唤醒 Worker（[证据:57](../../../src/tm/cpu_task.cpp)）。
- eu -> sched：Worker 循环从调度器运行队列取任务（[证据:246](../../../src/eu/cpu_worker.cpp)）。
- eu -> tm：Worker 调 `task->Execute()` 执行用户函数（[证据:184](../../../src/eu/cpu_worker.cpp)）。
- sync/dm -> eu：阻塞时走 `CoWait` 协程让出而非挂起线程（[证据:51](../../../src/sync/sleep.cpp)）。
- util -> dm/tm/sched/eu：`ffrt_init` 完成全部实现类的注册装配（[证据:52](../../../src/util/init.cpp)）。

## 一图看懂任务主链路

```text
ffrt_task_submit(f, ins, outs, attr)          # api
  └─> DependenceManager::onSubmit             # dm
        ├─ new SCPUEUTask(func_storage 前置)  # tm：任务对象即闭包容器
        ├─ ins/outs 签名 → Entity::VA2Ctx     # core：数据依赖图
        │    ├─ VersionCtx::AddConsumer       #     依赖未满足 → 挂在图上
        │    └─ VersionCtx::AddProducer
        └─ task->Ready()                      # 依赖满足（或无依赖）
              ├─ Scheduler::PushTask          # sched：按 QoS 入运行队列
              └─ ExecuteUnit::NotifyTask      # eu：唤醒 Worker
WorkerLooper: PopTask → task->Execute()       # eu → tm
  └─ f->exec(f)                               #     用户函数（协程中运行）
任务完成 → onTaskDone → 通知消费者 Ready       # dm/core
ffrt_wait / ffrt_mutex / sleep → CoWait       # sync：协程让出
```

## 建议阅读顺序

1. [api.md](modules/api.md)：先看用户视角有哪些 API、代码入口在哪。
2. [tm.md](modules/tm.md)：理解任务对象是“函数闭包 + 依赖计数 + 状态机”的合体。
3. [core.md](modules/core.md) + [dm.md](modules/dm.md)：这是 FFRT 的灵魂——数据依赖图。
4. [sched.md](modules/sched.md) + [eu.md](modules/eu.md)：任务怎么被调度和执行，协程何时让出。
5. [sync.md](modules/sync.md) + [queue.md](modules/queue.md)：阻塞语义与顺序约束。
6. [util.md](modules/util.md)：单例装配全貌，看懂 `ffrt_init`。
7. [dfx.md](modules/dfx.md)：维测面，按需查阅。

## 探索状态 / 遗留问题

| 项目 | 状态 | 说明 / 下一步 |
| --- | --- | --- |
| 提交主链路 api→dm→tm→sched→eu | 已验证 | 证据见上“文字版关键依赖” |
| 数据依赖图 Entity/VersionCtx | 已验证 | [modules/core.md](modules/core.md)；嵌套版本合并细节可再深入 |
| 队列任务分发 | 已验证 | [src/tm/queue_task.cpp:86](../../../src/tm/queue_task.cpp) |
| 帧调度（deadline/frame_interval） | 部分验证 | 头文件已读，与 QOS 边界可在接力时细化 |
| IPC 模块（src/ipc/ipc.cpp 仅 1 文件） | 待确认 | 单文件，疑似内部辅助，用途待查证 |
| 内核态硬化调度卸载 | 待确认 | README 提及，仓库内未见对应源码，可能在其他仓 |

## 相关文档

- 官方用户指南：[docs/README.md](../../../docs/README.md)
- 概念与并发范式：[docs/ffrt-overview.md](../../../docs/ffrt-overview.md)、[docs/ffrt-concurrency-paradigm.md](../../../docs/ffrt-concurrency-paradigm.md)
- 构建说明：[BUILD.md](../../../BUILD.md)、测试说明：[TESTING.md](../../../TESTING.md)
