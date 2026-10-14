# FFRT 参考手册（API 与源码索引）

> 本文是三件套的第三份：**[骨架篇](FFRT源码导读.md)管导航、[细节篇](FFRT实现细节.md)管机制、本篇管准确引用**。
> API 部分直接提取自仓库头文件（`interfaces/`），标注引入版本（since）；源码索引以函数/类名为主锚点（行号会漂移）。
> 本文不依赖项目文件即可阅读；官方文档路径仅在有仓库时用于进一步查阅。

---

## §1 公开 C API（interfaces/kits/c）

库：`libffrt.z.so` ｜ Kit：FunctionFlowRuntimeKit ｜ syscap：`SystemCapability.Resourceschedule.Ffrt.Core`

### 1.1 任务（task.h）

| API | since | 说明 |
| --- | --- | --- |
| `int ffrt_task_attr_init(ffrt_task_attr_t*)` | 10 | 初始化任务属性（默认 QoS） |
| `void ffrt_task_attr_destroy(ffrt_task_attr_t*)` | 10 | 销毁任务属性 |
| `void ffrt_task_attr_set_name(attr, const char*)` | 10 | 任务名（用于 trace/调试） |
| `const char* ffrt_task_attr_get_name(attr)` | 10 | 取任务名 |
| `void ffrt_task_attr_set_qos(attr, ffrt_qos_t)` | 10 | 设置 QoS |
| `ffrt_qos_t ffrt_task_attr_get_qos(attr)` | 10 | 取 QoS（默认 `ffrt_qos_default`） |
| `void ffrt_task_attr_set_delay(attr, uint64_t us)` | 10 | 延时提交；设置后 **in/out 依赖被忽略** |
| `uint64_t ffrt_task_attr_get_delay(attr)` | 10 | 取延时 |
| `void ffrt_task_attr_set_queue_priority(attr, ffrt_queue_priority_t)` | 12 | 并发队列任务优先级 |
| `ffrt_queue_priority_t ffrt_task_attr_get_queue_priority(attr)` | 12 | 取队列优先级 |
| `void ffrt_task_attr_set_stack_size(attr, uint64_t)` | 12 | 任务栈大小 |
| `uint64_t ffrt_task_attr_get_stack_size(attr)` | 12 | 取栈大小 |
| `void ffrt_task_attr_set_timeout(attr, uint64_t us)` | — | 调度超时，下限 100ms（低于即钳到 100ms） |
| `uint64_t ffrt_task_attr_get_timeout(attr)` | — | 取超时 |
| `void ffrt_task_attr_set_timeout_callback(attr, ffrt_function_header_t*)` | — | 超时回调 |
| `ffrt_function_header_t* ffrt_task_attr_get_timeout_callback(attr)` | — | 取超时回调 |
| `int ffrt_this_task_update_qos(ffrt_qos_t)` | 10 | 运行中动态改当前任务 QoS；0 成功，1 不可改（非普通任务/无映射） |
| `ffrt_qos_t ffrt_this_task_get_qos(void)` | 12 | 当前任务 QoS |
| `uint64_t ffrt_this_task_get_id(void)` | 10 | 当前任务 ID |
| `void* ffrt_alloc_auto_managed_function_storage_base(ffrt_function_kind_t)` | 10 | 分配闭包存储（运行时托管释放） |
| `void ffrt_submit_base(f, in_deps, out_deps, attr)` | 10 | 底层提交（无 handle） |
| `ffrt_task_handle_t ffrt_submit_h_base(f, in_deps, out_deps, attr)` | 10 | 底层提交（返回 handle） |
| `void ffrt_submit_f(func, arg, in_deps, out_deps, attr)` | 20 | 简化提交（无 destroy 回调） |
| `ffrt_task_handle_t ffrt_submit_h_f(func, arg, in_deps, out_deps, attr)` | 20 | 简化提交 + handle |
| `uint32_t ffrt_task_handle_inc_ref(handle)` | 12 | handle 引用 +1，返回旧值 |
| `uint32_t ffrt_task_handle_dec_ref(handle)` | 12 | handle 引用 -1，返回旧值 |
| `void ffrt_task_handle_destroy(handle)` | 10 | 销毁 handle |
| `void ffrt_wait_deps(const ffrt_deps_t*)` | 10 | 等指定依赖（数据/handle） |
| `void ffrt_wait(void)` | 10 | 等全部已提交任务 |
| `ffrt_error_t ffrt_set_worker_stack_size(qos, size_t)` | — | 设置某 QoS 的 Worker 栈大小 |
| `uint64_t ffrt_task_handle_get_id(handle)` | — | 由 handle 取任务 ID |

### 1.2 队列（queue.h）

| API | since | 说明 |
| --- | --- | --- |
| `int ffrt_queue_attr_init(ffrt_queue_attr_t*)` | 10 | 队列属性初始化 |
| `void ffrt_queue_attr_destroy(attr)` | 10 | 队列属性销毁 |
| `void ffrt_queue_attr_set_qos(attr, ffrt_qos_t)` | 10 | 队列 QoS |
| `ffrt_qos_t ffrt_queue_attr_get_qos(attr)` | 10 | 取队列 QoS |
| `void ffrt_queue_attr_set_timeout(attr, uint64_t us)` | 10 | 队列任务执行超时，下限 1ms |
| `uint64_t ffrt_queue_attr_get_timeout(attr)` | 10 | 取超时 |
| `void ffrt_queue_attr_set_callback(attr, f)` | 10 | 超时回调 |
| `ffrt_function_header_t* ffrt_queue_attr_get_callback(attr)` | 10 | 取超时回调 |
| `void ffrt_queue_attr_set_max_concurrency(attr, int)` | 12 | 并发队列最大并发 |
| `int ffrt_queue_attr_get_max_concurrency(attr)` | 12 | 取最大并发 |
| `void ffrt_queue_attr_set_thread_mode(attr, bool)` | 20 | true=线程模式（默认协程） |
| `bool ffrt_queue_attr_get_thread_mode(attr)` | 20 | 取执行模式 |
| `ffrt_queue_t ffrt_queue_create(type, name, attr)` | 10 | 创建串行/并发队列 |
| `void ffrt_queue_destroy(queue)` | 10 | 销毁：取消未启动任务，等待在执行任务 |
| `void ffrt_queue_submit(queue, f, attr)` | 10 | 队列提交 |
| `ffrt_task_handle_t ffrt_queue_submit_h(queue, f, attr)` | 10 | 队列提交 + handle |
| `void ffrt_queue_submit_f(queue, func, arg, attr)` | 20 | 简化队列提交 |
| `ffrt_task_handle_t ffrt_queue_submit_h_f(queue, func, arg, attr)` | 20 | 简化队列提交 + handle |
| `void ffrt_queue_submit_head(queue, f, attr)` | — | 提交到队头 |
| `ffrt_task_handle_t ffrt_queue_submit_head_h(queue, f, attr)` | — | 队头提交 + handle |
| `void ffrt_queue_wait(handle)` | 10 | 等队列任务完成 |
| `int ffrt_queue_cancel(handle)` | 10 | 取消未启动任务：0 成功 / 1 已执行或移除 / -1 空 handle |
| `ffrt_queue_t ffrt_get_main_queue(void)` | 12 | 应用主线程队列 |
| `ffrt_queue_t ffrt_get_current_queue(void)` | 12 | ArkTS worker 队列（**18 起废弃**） |
| `uint64_t ffrt_queue_get_task_cnt(queue)` | — | 队列当前任务数 |

队列类型：`ffrt_queue_serial` / `ffrt_queue_concurrent`。

### 1.3 互斥锁（mutex.h）

| API | since | 说明 |
| --- | --- | --- |
| `int ffrt_mutexattr_init(attr)` | 12 | 属性初始化 |
| `int ffrt_mutexattr_settype(attr, int type)` | 12 | normal / recursive |
| `int ffrt_mutexattr_gettype(attr, int*)` | 12 | 取类型 |
| `int ffrt_mutexattr_destroy(attr)` | 12 | 属性销毁 |
| `int ffrt_mutex_init(mutex, attr)` | 10 | 初始化（attr 可空） |
| `int ffrt_mutex_lock(mutex)` | 10 | 加锁 |
| `int ffrt_mutex_unlock(mutex)` | 10 | 解锁 |
| `int ffrt_mutex_trylock(mutex)` | 10 | 非阻塞尝试 |
| `int ffrt_mutex_destroy(mutex)` | 10 | 销毁 |

### 1.4 条件变量（condition_variable.h，since 10）

`ffrt_cond_init(cond, attr)` / `ffrt_cond_signal` / `ffrt_cond_broadcast` / `ffrt_cond_wait(cond, mutex)` / `ffrt_cond_timedwait(cond, mutex, timespec*)` / `ffrt_cond_destroy`

### 1.5 读写锁（shared_mutex.h，since 18）

`ffrt_rwlock_init(rwlock, attr)`（attr 当前仅支持 nullptr）/ `ffrt_rwlock_wrlock` / `ffrt_rwlock_trywrlock` / `ffrt_rwlock_rdlock` / `ffrt_rwlock_tryrdlock` / `ffrt_rwlock_unlock` / `ffrt_rwlock_destroy`

### 1.6 睡眠（sleep.h，since 10）

| API | 说明 |
| --- | --- |
| `int ffrt_usleep(uint64_t usec)` | 微秒级睡眠（协程任务中让出协程） |
| `void ffrt_yield(void)` | 让出执行权 |

### 1.7 定时器（timer.h，since 12）

| API | 说明 |
| --- | --- |
| `ffrt_timer_t ffrt_timer_start(qos, timeout_ms, data, cb, repeat)` | 在指定 QoS Worker 上起定时器；返回 -1 失败 |
| `int ffrt_timer_stop(qos, handle)` | 阻塞停止（勿在回调内调用，防死锁） |

### 1.8 事件循环（loop.h，since 12）

| API | 说明 |
| --- | --- |
| `ffrt_loop_t ffrt_loop_create(queue)` | 在队列上建 loop |
| `int ffrt_loop_destroy(loop)` | 销毁 |
| `int ffrt_loop_run(loop)` | 占用调用线程同步跑 loop |
| `void ffrt_loop_stop(loop)` | 停止 |
| `int ffrt_loop_epoll_ctl(loop, op, fd, events, data, cb)` | loop 上的 epoll 增删改 |
| `ffrt_timer_t ffrt_loop_timer_start(loop, timeout_ms, data, cb, repeat)` | loop 定时器 |
| `int ffrt_loop_timer_stop(loop, handle)` | 停 loop 定时器 |

### 1.9 协程 Fiber（fiber.h，since 20）

| API | 说明 |
| --- | --- |
| `int ffrt_fiber_init(fiber, func, arg, stack, stack_size)` | 初始化用户态纤程（调用方管理栈） |
| `void ffrt_fiber_switch(from, to)` | 保存 from 上下文、切换到 to |

### 1.10 关键类型（type_def.h）

**QoS 枚举**：

| 值 | 含义 | since |
| --- | --- | --- |
| `ffrt_qos_inherit = -1` | 继承调用线程 QoS | 10 |
| `ffrt_qos_background` | 后台（最低） | 10 |
| `ffrt_qos_utility` | 长期可见任务 | 10 |
| `ffrt_qos_default` | 默认 | 10 |
| `ffrt_qos_user_initiated` | 用户发起、需快响应 | 10 |
| `ffrt_qos_deadline_request` | 有明确截止时间 | 23 |
| `ffrt_qos_user_interactive` | 用户交互（最高） | 23 |
| `ffrt_qos_max = user_interactive` | 最大值 | 23 |

**并发队列优先级**（since 12）：`immediate(0)` > `high` > `low` > `idle`。immediate 立即派发，其余按 handle 时间排序。

**依赖类型**：`ffrt_dependence_data`（数据就绪）/ `ffrt_dependence_task`（任务完成）。依赖结构 `ffrt_deps_t{len, items[]}`，每项 `ffrt_dependence_t{type, ptr}`。

**闭包头**：`ffrt_function_header_t{exec, destroy, reserve[2]}`，存储大小 `ffrt_auto_managed_function_storage_size = 64 + sizeof(header)`。

**错误码**：`ffrt_success=0`、`ffrt_error=-1`、`ffrt_error_nomem=ENOMEM`、`ffrt_error_timedout=ETIMEDOUT`、`ffrt_error_busy=EBUSY`、`ffrt_error_inval=EINVAL`。

---

## §2 C++ API（interfaces/kits/cpp）

C++ 侧是类型安全封装，全部落到 §1 的 C API：

| 类/函数（ffrt 命名空间） | 对应 C API | 要点 |
| --- | --- | --- |
| `task_attr` | `ffrt_task_attr_*` | RAII 构造/析构；链式 set |
| `task_handle` | handle inc/dec | 引用计数自动管理 |
| `submit(func, in_deps, out_deps, attr)` | `ffrt_submit_base` | 模板闭包包装 |
| `submit_h(...)` | `ffrt_submit_h_base` | 返回 task_handle |
| `wait()` / `wait_deps(deps)` | `ffrt_wait*` | — |
| `queue` / `queue_attr` | `ffrt_queue_*` | submit / submit_head / wait；`submit_apply` 批量提交 |
| `mutex` / `recursive_mutex` | `ffrt_mutex_*` | fast path：CAS 先试，失败走 lock_wait |
| `condition_variable` | `ffrt_cond_*` | wait / wait_for / wait_until（模板谓词版） |
| `shared_mutex` | `ffrt_rwlock_*` | — |
| `this_task::sleep_for(d)` / `yield()` | `ffrt_usleep` / `ffrt_yield` | chrono 时长 |
| `pattern/job_partner` | 基于 fiber | 主从协作 job，master 等待时可帮跑 partner 任务 |
| `pattern/job_ring` | 基于 fiber | 环形 job 队列 |
| `pattern/job_utils` | fiber/atomic 工具 | futex、SPMC/MPSC 队列、job promise 等无锁原语 |

---

## §3 内部接口（interfaces/inner_api，系统部件使用）

按域列出（C 接口为主）：

| 域 | 主要接口 |
| --- | --- |
| deadline（帧调度） | `ffrt_interval_create/update/begin/end/destroy/join/leave` |
| dump | `ffrt_dump(cmd, buf, len)`；任务超时回调 get/set、阈值 get/set；异步栈注册 `FFRTSetAsyncStackFunc` |
| cpu_boost | `ffrt_cpu_boost_start/end(ctx_id)` |
| ipc | `ffrt_this_task_set_legacy_mode(bool)` |
| thread | `ffrt_thread_create/join/detach` |
| executor_task | `ffrt_executor_task_register_func/submit/cancel`；`ffrt_epoll_ctl/wait/get_count/get_wait_time`；`ffrt_poller_wakeup`；`ffrt_submit_coroutine/wake_coroutine`；`ffrt_get_current_task/get_cur_task`；`ffrt_task_attr_set/get_local`（task-local）；`ffrt_task_get_tid` |
| task_ext | `ffrt_skip(handle)`（跳过未执行任务）；`ffrt_set_cgroup_attr`；`ffrt_set_cpu_worker_max_num`；`ffrt_task_attr_set_notify_worker`；`ffrt_notify_workers`；`ffrt_enable/disable_worker_escape`；`ffrt_set_sched_mode` |
| queue_ext | `ffrt_queue_has_task/cancel_all/cancel_and_wait/cancel_by_name/is_idle/dump/size_dump`；`ffrt_queue_set_eventhandler`；`ffrt_concurrent_queue_wait_all` |
| mutex_ext | `ffrt_mutex_lock_wait` / `ffrt_mutex_unlock_wake`（协程拆分锁） |
| init | `ffrt_child_init`（子进程 clone 场景初始化 FFRT 资源，仅可调用一次） |

---

## §4 机制 → 源码索引

每条机制给出**函数/类名锚点**（首选）与行号（次选，2026-10-04 快照）。与细节篇章节对应。

| 机制 | 源码锚点 | 细节篇 |
| --- | --- | --- |
| 装配入口 | `ffrt_init` @ `src/util/init.cpp:52`（constructor） | §9.1 |
| 单例门面 | `FFRTFacade` / `SINGLETON_LIST` @ `src/util/ffrt_facade.h` | §9.2 |
| 提交入口 | `submit_impl` @ `src/core/task.cpp:76` | §1.1 |
| 提交编排 | `SDependenceManager::onSubmit` @ `src/dm/sdependence_manager.cpp:68` | §1.2 |
| handle 输出签名 | onSubmit 内 `outsNoDup.push_back(handle)` @ 同上 | §1.3 |
| 签名映射 | `Entity::VA2Ctx` @ `src/core/entity.cpp:21` | §2.1 |
| 版本节点 | `VersionCtx` @ `src/core/version_ctx.h`；`onProduced` @ `version_ctx.cpp:71`；`onConsumed` @ `:92` | §2.3 |
| 嵌套版本合并 | `VersionCtx::CreateChildVersion / MergeChildVersion` @ `version_ctx.cpp` | §2.4 |
| 任务基类 | `TaskBase` @ `include/tm/task_base.h`（生命周期虚接口） | §3.1 |
| 闭包前置存储 | `GetCPUTaskByFuncStorageOffset` @ `include/tm/cpu_task.h` | §3.2 |
| 就绪入队 | `CPUEUTask::Ready` @ `src/tm/cpu_task.cpp:49` | §1.4 |
| 执行互斥 | `skipped` CAS @ `src/tm/cpu_task.cpp:85`（Execute） | §3.4 |
| done 延迟解依赖 | `coRoutine->isTaskDone` @ `src/tm/cpu_task.cpp:121` | §3.4 |
| per-QoS 门面 | `Scheduler` @ `include/sched/scheduler.h` | §4.1 |
| 默认调度实现 | `STaskScheduler` @ `src/sched/stask_scheduler.h` | §4.2 |
| UV 并发控制 | `uvTaskWaitingQueue_ / cancelSet_` @ `include/sched/task_scheduler.h` | §4.3 |
| UV/executor 回流 | `ffrt_executor_task_register_func / submit` @ `src/core/task.cpp:630-660`；批量收割 `UVTask::ExecuteImpl` @ `src/tm/uv_task.cpp` | §4.3 |
| 帧调度 / 截止时间 | `ffrt_interval_create/begin/end` @ `src/sched/deadline.cpp:57-106`；`Interval / DefaultInterval / FrameInterval` @ `src/sched/interval.h`、`frame_interval.h` | §4.4 |
| Worker 主循环 | `CPUWorker::WorkerLooper` @ `src/eu/cpu_worker.cpp:239` | §5.3 |
| Worker 取任务 | `schedIns.PopTask` @ `src/eu/cpu_worker.cpp:246` | §5.3 |
| Worker 组 | `CPUWorkerGroup` @ `include/eu/execute_unit.h` | §5.1 |
| 协程底座 | `co2_context.c`（上下文切换汇编）、`CoRoutine` @ `include/eu/co_routine.h` | §5.4 |
| fd 轮询 | `IOPoller` @ `src/eu/io_poller.cpp`；缓存清理 `ClearCachedEvents` | §5.5 |
| QoS 内核接口 | `src/eu/qos_interface.cpp`；RTG：`src/eu/rtg_ioctl.cpp` | §5.6 |
| mutex | `ffrt_mutex_lock` @ `src/sync/mutex.cpp:354`；协程锁拆分 @ `mutex.cpp:243` | §6.3 |
| sleep | `SleepUntilImpl` / `CoWait` @ `src/sync/sleep.cpp:36-51` | §6.2 |
| 等待队列底座 | `WaitQueue` @ `src/sync/wait_queue.cpp` | §6.1 |
| 定时器 | `TimerManager` @ `src/sync/timer_manager.cpp`；`DelayedWorker` @ `src/sync/delayed_worker.cpp` | §6.2 |
| 队列创建 | `ffrt_queue_create` @ `src/queue/queue_api.cpp:196`；提交 @ `:215` | §7.1 |
| 队列任务就绪 | `QueueTask::Ready` @ `src/tm/queue_task.cpp:80` | §7.2 |
| 队列监控 | `QueueMonitor` / `TrafficRecord` @ `src/queue/` | §7.3 |
| 队列过载诊断 | `TrafficRecord::CalculateTraffic` @ `src/queue/traffic_record.cpp`（subcnt/doncnt/过载判定） | §7.4 |
| HiTrace 标记协议 | `FFRT_SUBMIT_MARKER` 等 @ `include/dfx/trace/ffrt_trace.h`；chain @ `src/dfx/trace/ffrt_trace_chain.cpp` | §8.1 |
| 分级统计 | `FFRTTraceRecord` @ `include/dfx/trace_record/ffrt_trace_record.h` | §8.2 |
| 任务看门狗 | `SendTimeoutWatchdog` @ `src/dfx/watchdog/watchdog_util.cpp:48`；注册 @ `sdependence_manager.cpp:116`，移除 @ `cpu_task.cpp:93` | §8.3 |
| Worker 周期监控 | `CheckWorkerStatus` @ `src/util/worker_monitor.cpp:152`；`CheckTaskStatus` @ `:199`；IPC 卡死 `RecordIpcInfo` @ `:408` | §8.4 |
| 运行时快照 | `ffrt_dump` @ `src/dfx/dump/dump.cpp:157`；暂停协程回溯 `DumpTask` @ `:62` | §8.5 |
| 黑盒 | `SaveTheBbox` @ `src/dfx/bbox/bbox.cpp:397`；信号处理 @ `:471`；冻结入口 `BboxCheckAndFreeze`（调用点 `cpu_task.cpp:63`） | §8.6 |
| 事件上报 | `TaskTimeoutReport` / `WorkerEscapeReport` 等 @ `src/dfx/sysevent/sysevent.cpp:146-205`；事件定义 `hisysevent.yaml` | §8.7 |
| 日志体系 | `include/dfx/log/ffrt_log_api.h`（分级/Beta 门控/隐私标注） | §8.8 |
| 异步栈钩子 | `FFRTSetAsyncStackFunc` @ `src/dfx/async_stack/ffrt_async_stack.cpp`；切入时机 `co_routine.cpp:464` | §8.9 |
| trace 工具 | `tools/ffrt_trace_process/`（recover / process / GUI） | §8.10 |
| 对象池 | slab @ `include/util/slab.h` | §9.3 |
| 依赖环检测 | `graph_check.cpp` | §9.3 |
| 退出路径 | `ProcessExitManager` @ `src/util/ffrt_facade.cpp`；`~SExecuteUnit` @ `src/eu/sexecute_unit.cpp:79`；`DelayedWorker::Terminate` @ `src/sync/delayed_worker.cpp:235` | §9.4 |

---

## §5 官方文档导读（仓库 docs/）

有仓库时按需查阅（本表为内容地图，非链接）：

| 文档 | 内容 | 何时读 |
| --- | --- | --- |
| `docs/ffrt-overview.md` | 概念、线程 vs 任务模型对比、三大特性 | 入门第一篇 |
| `docs/ffrt-concurrency-paradigm.md` | 并发范式详解 | 概念不清时 |
| `docs/ffrt-api-guideline-c.md` | **官方 C API 权威参考** | 查 API 语义/版本 |
| `docs/ffrt-api-guideline-cpp.md` | 官方 C++ API 参考 | 同上 |
| `docs/ffrt-development-guideline.md` | 开发指导、最佳实践 | 写业务代码前 |
| `docs/ffrt-concurrency-graph-c(-cpp).md` | 图依赖示例 | 学数据依赖 |
| `docs/ffrt-concurrency-serial-queue-c(-cpp).md` | 串行队列示例 | 学队列 |
| `docs/ffrt-concurrency-concurrent-queue-c(-cpp).md` | 并发队列示例 | 学队列 |
| `docs/ffrt-concurrency-job-partner-cpp.md` | job_partner 模式 | 高性能场景 |
| `docs/ffrt-example-kotlin.md` | Kotlin 侧用法 | 跨语言集成 |
| `BUILD.md` / `TESTING.md` / `BENCHMARKS.md` / `RELEASE.md` | 构建 / 测试 / 性能 / 版本说明 | 工程化时 |

---

## §6 版本兼容速查（since 汇总）

| 版本 | 引入能力 |
| --- | --- |
| 10 | 核心：任务提交/等待、任务属性（name/qos/delay）、队列、mutex、cond、sleep |
| 12 | 队列优先级、任务栈大小、handle 引用计数、timer、loop、主队列、mutexattr、this_task_get_qos |
| 18 | 读写锁（rwlock）；`ffrt_get_current_queue` 废弃 |
| 20 | 简化提交（submit_f/submit_h_f）、队列线程模式、fiber |
| 23 | QoS 扩档：deadline_request、user_interactive、qos_max |

---

## §7 宏开关速查

| 宏 | 作用 |
| --- | --- |
| `USE_OHOS_QOS` | 系统 QoS 接口（否则 staging 实现） |
| `QOS_WORKER_FRAME_RTG` | RTG 线程组 |
| `FFRT_CLOSE_COROUTINE_MODE` | 关协程（TSAN 自动启用） |
| `FFRT_OH_TRACE_ENABLE` | HiTrace |
| `FFRT_TRACE_RECORD_LEVEL` | 事件记录级别 |
| `FFRT_ASYNC_STACKTRACE` | 异步调用栈 |
| `FFRT_BBOX_ENABLE` | 黑盒冻结 |
| `FFRT_TASK_LOCAL_ENABLE` | 任务级 thread-local |
| `FFRT_WORKERS_DYNAMIC_SCALING` | Worker 动态伸缩 |
| `FFRT_SEND_EVENT` | Beta 版本事件 |

构建：OHOS 用 GN（`BUILD.gn`），通用平台用 CMake；导出符号由 `libffrt.map` 约束。

→ 返回骨架篇：**[FFRT源码导读.md](FFRT源码导读.md)** ｜ 机制深挖：**[FFRT实现细节.md](FFRT实现细节.md)**
