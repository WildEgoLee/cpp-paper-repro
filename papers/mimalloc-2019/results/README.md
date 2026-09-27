# 结果

入库的文件必须自己说明身份。`runs/` 下面的原始多次输出不入库。

## environment-paper.json

身份：**APLAS / MSR-TR-2019-18 的历史锁定。不是一次运行结果。**

正文机器是 EC2 `r5a.4xlarge`（16 核 EPYC 7000，2.5 GHz，128 GB，Ubuntu 18.04.1，LibC 2.27，GCC 7.3.0）和 HP Z4-G4（4 核 Xeon W-2123，3.6 GHz，16 GB，同一套系统软件）。图里嵌的「8 核 @2.7 GHz、GCC 7.4.0」以冲突项保留，不当成第二套官方机器。

分配器钉的是论文写的那三个，不是后来脚本里的版本：

| id | 锁定 |
| --- | --- |
| mi | tag `v1.0.0`，commit `1125271c2756ee1db1303918816fea35e08b3405` |
| je | tag `5.2.0`，commit `b0b3e49a54ec29e32636f4577d9d5a896d67fd20` |
| tc | Ubuntu 源码包 `google-perftools 2.5-2.2ubuntu3`，不是 git tag `gperftools-2.7` |

mimalloc-bench 锁在 `874d1b837ebf590eca732bc916bc63b2c1ebffd4`（2019-06-20）。这一版 README 和论文 §4 一致，构建脚本仍安装 Ubuntu 的 tcmalloc 包。下一周的脚本改去编译 `gperftools-2.7`，锁定文件拒绝那个提交。这个提交里的脚本 checkout 的是 `dev`，不是 `v1.0.0`；复现构建以 tag 为准。

正式 96 组还没跑。跑之前的机器要至少有 12 个硬件线程，并记下 SMT、NUMA、governor、turbo。下面那份 2 线程探针不够格。

## sandbox-mechanism.json

身份：**机制探针，一次运行。不是 APLAS 2019 的复现。**

| 项 | 值 |
| --- | --- |
| 机器 | Intel Xeon Platinum 8481C @ 2.70GHz，2 个硬件线程，1 核 1 线程 |
| 系统 | Linux 6.12.8+，x86_64 |
| 编译器 | g++ 12.2.0，`-O2 -std=c++17` |
| 重复 | 每个变体 7 次，表内为中位数 |

这不是论文的 EPYC / GCC 7 / glibc 2.27 环境。只有 2 个硬件线程，不能用来谈 12 线程扩展性。

中位数（越小越好）：

| 实验 | 变体 | 中位数 |
| --- | --- | --- |
| locality_walk | 页内顺序 | 8.07 ns/visit |
| locality_walk | 跨页步长 | 123.3 ns/visit |
| fast_path | free-list pop | 1.71 ns/op（4.61 cycles） |
| fast_path | bump pointer | 1.34 ns/op（3.63 cycles） |
| temporal_cadence | 每次分配都做维护 | 10.93 ns/alloc |
| temporal_cadence | free list 空了才做 | 4.53 ns/alloc |
| cross_thread_free | 全局 mutex | 88.5 ns/op |
| cross_thread_free | 页局部原子 thread_free | 8.94 ns/op |
| false_sharing | 同一条 cache line | 35.5 ns/increment |
| false_sharing | 分开的 cache line | 6.29 ns/increment |

在这台机器上，方向是：

- 页内顺序的后续追逐远便宜于跨页 free list。这支持 C3 的机制，但不等于 Lean 或 redis 的论文数字。
- 隔离的 bump pointer 比 L1 free-list pop 更快。论文「不用 bump」的理由是完整分配器里的分支，不是这个微基准。Exact 档这里 **不支持** 把微基准读成「pop 比 bump 快」。
- 同样的检查放到 slow path 上，分配更快。支持 cadence 的机制。
- 页局部原子栈远快于一把全局锁。对照物不是 jemalloc，不能写成 C2 已复现。原子栈那一列的 7 次里有几次明显偏慢（约 20 ns），中位数仍在 9 ns 附近；报告时保留样本，不要只留中位数。
- 两个写者挤在同一条 cache line 上更慢。支持 C4 的机制，不是论文里的十几倍。

C1、C5、以及针对 jemalloc / tcmalloc / glibc 的 Directional 结论：**还没有数据。**
