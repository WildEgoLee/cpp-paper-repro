# 机制探针

不链接 mimalloc。用几个小模型把论文里的机制从完整分配器里拆出来。

```bash
make
./mechanism_probe            # JSON → stdout
make test                    # 跨线程路径是否丢块
```

需要 GCC 或 Clang，C++17，pthread。`-O2` 是协议的一部分：`freelist_pop` 和 `bump_wrap` 标了 `noipa`，避免编译器把一条看得见的小环消掉。不要用 `-O0` 的数字和 `-O2` 的数字比。

| 实验 id | 两个变体在比什么 |
| --- | --- |
| `locality_walk` | 64MiB 上按分配顺序追逐。顺序落在 64KiB 页内，对照是步长 1025 个 64B 节点（与 2^20 互素，且刚好比一页大） |
| `fast_path` | L1 里的 free-list pop，对照 64KiB 页上的 bump pointer。bump 在这个隔离测试里更快，并不反驳论文：论文说的是完整分配器里多一个分支，不是「指针追逐比加法慢」 |
| `temporal_cadence` | 同样的 4 次 relaxed 原子加载，要么每次 pop 都做，要么只在页内 free list 变空时做 |
| `cross_thread_free` | 两个线程互相释放对方的块。全局 `mutex`，对照页局部带 tag 的原子栈。tag 假设用户态指针的高 16 位为 0；不满足时探针直接退出 |
| `false_sharing` | 两个线程各加自己的 `atomic<uint64_t>`。相邻，或各自独占一条 cache line |

自我检查会再跑一遍跨线程路径并清点节点。`--self-test` 使用和正式运行相同的轮数，所以它本身就要几秒。
