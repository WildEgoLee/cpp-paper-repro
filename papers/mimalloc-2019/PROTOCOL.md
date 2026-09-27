# 协议：mimalloc-2019

状态：`protocol-frozen`。冻结日期以本文件进入仓库的提交为准。下面的判据在跑完之前不再改写。

论文环境以 PDF 的实验设置表为准。工作笔记里出现过的机器是：AMD EPYC（16 核一类的配置，Ubuntu 18.04，glibc 2.27，GCC 7.x）以及 Intel Xeon W-2123。**开跑之前对照 PDF 把型号、核数、内存和编译器版本抄进 `results/` 的机器说明。** 不要凭笔记填 Exact 档。

## 已经从论文公开文本核对过的事实

这些可以写进协议，不必再等 PDF：

- 评测的 mimalloc 是 tag **v1.0.0**。另有一个 secure 变体，不进入第一轮。
- 软件页大约 **64KiB**（64 位），页内是同一个 size class。
- 每个页三条链表：`free`、`local_free`、`thread_free`。跨线程只用原子操作，不拿全局锁。
- temporal cadence：fast path 保持很短，维护放在必然会发生的 slow path 上。
- 作者试过空页上的 bump pointer，为了少一个 fast-path 分支而没有采用。工作笔记里的「大约慢 2%」在对照 PDF 之前只记成待核对，不作为 Exact 目标。
- redis 使用 **Redis 5.0.3**，100 万次请求。公开文本写 mimalloc 在该测试上比 jemalloc 快 14%；摘要把相对 tcmalloc 和 jemalloc 的加速写成 7% 和 14%。
- larsonN：每个线程分配并释放大量对象，同时留下一部分给别的线程释放。公开文本写大约 100 个线程，mimalloc 比 tcmalloc 和 jemalloc 快 **2.5 倍以上**，作者将其联系到跨线程对象迁移。
- 论文同时报告相对时间和相对 peak RSS，并承认有 mimalloc 并不占优的 workload。RSS 失控算失败，不是「跑得快就行」。

其他 allocator 的精确版本从论文评测节或与 `v1.0.0` 同时代的 mimalloc-bench 提交里抄。不要用后来 README 上的 jemalloc 5.2 / tcmalloc 版本冒充 2019 年的对照。

## Claim

五条都要能失败。

### C1 通用性能

在 glibc、jemalloc、tcmalloc 旁边，mimalloc `v1.0.0` 在第一轮 workload 的多数配置上不显著更慢。

失败：在单线程 alloc-test 或 redis 上稳定地明显慢于 glibc 和 jemalloc（中位数差距大于重复运行的散布，且方向在 15 次里稳定）。

### C2 跨线程分配 / 释放

larson 与 xmalloc 是优势区，不是通用榜上的一个普通格子。

失败：这两个 workload 上，mimalloc 相对 jemalloc 和 tcmalloc 没有稳定的正向差距。不要求复现「2.5×」这个 Exact 数字。

### C3 局部性溢出到分配器外面

分配器造成的空间局部性会改变后来的访问，而不只是 malloc 本身的耗时。

失败：sharded 分配与跨页 strided free list 在随后的指针追逐上没有稳定差距，而且完整 workload 的 cache miss / IPC 也没有同方向的变化。只看到 malloc 吞吐变高，不算这条成立。

### C4 false sharing

页归一个线程所有时，分配器本身把同时写入的对象放进同一条 cache line 的机会下降。

失败：cache-scratch 形态下，mimalloc 与 jemalloc/tcmalloc 的 cache miss 或运行时间没有稳定差距。论文里出现过的十几倍差距 **不是** 必须命中的数字。

### C5 内存

更快的同时，peak RSS 不相对基线大幅升高。

失败：时间变好，但 peak RSS 相对 glibc 或 jemalloc 稳定地成倍上升。论文承认的个别 workload 落后要单独列出，不能从总表里删掉。

## 第一轮只跑这四组

| workload | 看什么 |
| --- | --- |
| alloc-test | 基础分配吞吐。Pareto 大小分布；含多线程版本 |
| larsonN | 跨线程 free，对象迁移 |
| xmalloc-testN | producer / consumer 极端迁移 |
| redis | Redis 5.0.3，100 万请求 |

不在第一轮：cache-scratch 以外的全部 SPEC、secure mimalloc、Hoard、snmalloc、rpmalloc、TBB、Windows。cache-scratch 属于 C4，作为第二批，不塞进下面这 96 组。

## 矩阵

4 个分配器 × 6 个线程数 × 4 个 workload = **96** 组。定义在 [bench/matrix.json](bench/matrix.json)。

```text
分配器     glibc, mimalloc v1.0.0, jemalloc, tcmalloc
线程       1, 2, 4, 6, 8, 12
workload   alloc-test, larson, xmalloc, redis
预热       3
正式       15
统计       median, p95
```

`mimalloc-current` 不进入这 96 组。它是另一条轨道，重复同一矩阵，单独报告。

每次正式运行记录：

```text
wall time, throughput
median, p95
RSS, peak RSS
cycles, instructions, IPC
L1-dcache-load-misses, LLC-load-misses, cache-misses
context-switches
```

Linux 上用 `perf stat`。没有 perf 的机器可以先交墙钟和 RSS，但 C3/C4 不能只靠墙钟结案。

## Ablation

在 `v1.0.0` 上做，不做在一个已经改了很多年的树顶上。三个开关：

| 关掉 | 预期 |
| --- | --- |
| A. page-local sharding，退回该 size class 一条大 free list | 顺序分配后的访问变差；跨线程也变差 |
| B. `local_free`，本线程 free 直接回到 `free` | 顺序路径变差或 fast path 变长；跨线程不是主要损失 |
| C. `thread_free`，跨线程 free 改走共享结构 | 顺序路径几乎不变；larson / xmalloc 明显变差 |

```text
                  顺序        跨线程
sharding          变好        变好
local_free        变好        不明显
thread_free       不明显      明显变好
```

若开关做不到源码级干净切断，就在 `harness/` 里用同一工作负载对比「有 / 没有该机制」的模型，并在结果里写明这是模型 ablation，不是 `v1.0.0` 的源码 ablation。两种不能混在一张表里。

## 机制探针已经覆盖的部分

`harness/mechanism_probe` 是模型，不是 `v1.0.0`。它对应：

| 探针实验 | claim |
| --- | --- |
| locality_walk | C3 的机制部分 |
| fast_path | bump 与 free-list pop 的隔离比较；不能用来判定论文里的 2% |
| temporal_cadence | `local_free` / slow path 的机制部分 |
| cross_thread_free | C2 的机制部分，对照物是全局 mutex，不是 jemalloc |
| false_sharing | C4 的机制部分，对照物是同一条 cache line，不是 jemalloc |

探针通过，只说明 Mechanistic 档里「这些机制在这台 CPU 上仍然有方向」。它不能把 C1–C5 标成已复现。

## 验收

跑完后按这个表填，不允许临时改判据。

| Claim | 复现判据 |
| --- | --- |
| C1 通用性能 | 多数 workload 上不显著慢于 glibc / jemalloc / tcmalloc |
| C2 跨线程 | larson 与 xmalloc 有稳定优势 |
| 扩展性 | 线程增加时，吞吐扩展至少好过其中一部分对照 |
| C3 局部性 | cache miss 或 IPC 有一致方向的改善，而不只是 malloc 更快 |
| C5 内存 | RSS 不因为时间变好而大幅变差 |
| ablation | 关掉的机制击中上表预测的那一列 |

另外单独写三行：Exact、Directional、Mechanistic。三行都可以是「不支持」。
