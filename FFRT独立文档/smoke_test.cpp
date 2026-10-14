// FFRT 冒烟验证：在有源码与构建产物时运行，验证文档中的核心行为断言。
// 构建：g++ -std=c++17 smoke_test.cpp -I<repo>/interfaces/kits -I<repo>/interfaces/inner_api \
//        -I<repo>/include -L<build>/src -lffrt -lpthread -ldl -o smoke_test
// 运行：LD_LIBRARY_PATH=<build>/src ./smoke_test
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include "ffrt.h"
#include "cpp/task.h"
#include "cpp/queue.h"
#include "cpp/sleep.h"

int main()
{
    int failures = 0;
    auto check = [&](bool ok, const char* name) {
        std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", name);
        if (!ok) {
            failures++;
        }
    };

    // 1. RAW 依赖链：A 写 a → B 读 a 写 b → C 读 b（细节篇 §2.7 走例）
    {
        int a = 0;
        int b = 0;
        std::atomic<int> rawOk { 0 };
        ffrt::submit([&] { a = 1; }, {}, { &a });
        ffrt::submit([&] {
            if (a == 1) {
                rawOk++;
            }
            b = 2;
        }, { &a }, { &b });
        ffrt::submit([&] {
            if (b == 2) {
                rawOk++;
            }
        }, { &b }, {});
        ffrt::wait();
        check(rawOk == 2 && a == 1 && b == 2, "RAW dependency chain (A -> B -> C)");
    }

    // 2. WAR：先提交读者后提交写者，读者必须看到旧值（提交顺序无关性）
    {
        int x = 100;
        const int readers = 4;
        std::atomic<int> seenOld { 0 };
        for (int i = 0; i < readers; i++) {
            ffrt::submit([&] {
                if (x == 100) {
                    seenOld++;
                }
            }, { &x }, {});
        }
        ffrt::submit([&] { x = 200; }, {}, { &x });
        ffrt::wait();
        check(seenOld == readers && x == 200, "WAR: pre-submitted readers see old value");
    }

    // 3. 串行队列：100 个任务严格按提交顺序执行
    {
        ffrt::queue q { ffrt::queue_type::queue_serial, "smoke_serial" };
        std::atomic<int> seq { 0 };
        std::atomic<int> firstBad { -1 };
        std::atomic<bool> ordered { true };
        std::atomic<bool> drain { false };
        for (int i = 0; i < 100; i++) {
            q.submit([&, i] {
                if (seq.load() != i) {
                    ordered = false;
                    int expect = -1;
                    firstBad.compare_exchange_strong(expect, i);
                }
                seq++;
            });
        }
        q.submit([&] { drain = true; });
        while (!drain.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        check(ordered && seq == 100, "serial queue FIFO order (100 tasks)");
        std::printf("       (serial queue: firstViolation=%d, finalSeq=%d)\n",
            firstBad.load(), seq.load());
    }

    // 4. 协程 sleep：8 个各睡 50ms 的任务应并行，总耗时远小于 400ms
    {
        std::atomic<int> concurrent { 0 };
        std::atomic<int> maxConcurrent { 0 };
        auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < 8; i++) {
            ffrt::submit([&] {
                int c = ++concurrent;
                int m = maxConcurrent.load();
                while (c > m && !maxConcurrent.compare_exchange_weak(m, c)) {
                }
                ffrt::this_task::sleep_for(std::chrono::milliseconds(50));
                --concurrent;
            }, {}, {});
        }
        ffrt::wait();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
        std::printf("       (parallel sleep: maxConcurrent=%d, total=%lldms)\n",
            maxConcurrent.load(), static_cast<long long>(ms));
        check(maxConcurrent.load() > 1, "coroutine sleep frees workers (parallelism observed)");
        check(ms < 400, "8 x sleep(50ms) not serialized");
    }

    // 5. 嵌套任务 + ffrt::wait join 语义
    {
        std::atomic<int> done { 0 };
        ffrt::submit([&] {
            for (int i = 0; i < 4; i++) {
                ffrt::submit([&] { done++; }, {}, {});
            }
            ffrt::wait(); // 等全部子任务
        }, {}, {});
        ffrt::wait();
        check(done == 4, "nested tasks + ffrt::wait join semantics");
    }

    std::printf("==== %s (%d failure%s) ====\n",
        failures == 0 ? "ALL PASS" : "FAILURES", failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
