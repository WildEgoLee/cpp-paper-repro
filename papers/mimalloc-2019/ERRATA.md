# 勘误

这些条目不改 [PROTOCOL.md](PROTOCOL.md) 里已经写下的失败判据。C4 的失败条件一个字不动。这里只补冻结时漏掉的验收行，以及 96 组到底能关闭什么。

版本和机器的机器可读锁定在 [results/environment-paper.json](results/environment-paper.json)。那份文件是执行事实，不是新的 claim。

## E1 验收表补上 C4

PROTOCOL 末尾的验收表有 C1、C2、扩展性、C3、C5、ablation，没有 C4。C4 的失败判据仍是原文：

> cache-scratch 形态下，mimalloc 与 jemalloc/tcmalloc 的 cache miss 或运行时间没有稳定差距。论文里的十几倍（正文 §4.3 写的是超过 18×）不是必须命中的数字。

补进验收表的一行是：

| Claim | 复现判据 |
| --- | --- |
| C4 false sharing | 同上，不另写一套。cache-scratch 不在第一轮 96 组里，所以这 96 组无论结果如何都不能把 C4 打勾 |

## E2 96 组不能当成五条 claim 全部结案

`96 × (3 warmup + 15 measured) = 1728` 次 workload 调用。不算在里面的有：mimalloc-current、larson 的论文 100 线程点、xmalloc 的 100+100、ablation、cache-scratch、secure mimalloc。

| 条目 | 这 1728 次能关闭吗 |
| --- | --- |
| C1 通用性能 | 能。范围只有 alloc-test、larson、xmalloc、redis |
| C2 跨线程 | 能做 Directional。论文那个 100 线程的 larson 点不在这 96 组里，要另跑 |
| 扩展性 | 能。前提是机器至少有 12 个硬件线程。少于这个数目，6/8/12 线程测到的是超订，不是分配器扩展 |
| C5 内存 | 能。只覆盖这四项 workload 的 RSS |
| C3 局部性 | 不能结案。96 组最多提供这四项 workload 的 perf 计数器。还要对上 locality / ablation 的方向 |
| C4 false sharing | 不能。必须等第二批 cache-scratch |

96 组跑完，不等于五条 claim 都复现了。

## E3 核对过、但没有变成新判据的事实

MSR-TR-2019-18 §2.2 写明：试过带 bump pointer 的变体，整个 benchmark 上大约慢 2%，原因是 fast path 上变成两个条件。PROTOCOL 里「待核对、不作为 Exact 目标」的前半句已经核对完。后半句保持不变：第一轮不建这个变体，2% 不是 Exact 目标。

harness 里 L1 上的 bump 比 free-list pop 快，是另一个实验。不能用它去勾或否 §2.2。
