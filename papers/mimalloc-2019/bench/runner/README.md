# runner

P1 的执行语义。这一版只有 preflight 和 dry-run。没有 execute，也没有 benchmark 数字。

冻结矩阵在 [../matrix.json](../matrix.json)，没有改。`logical_threads` 不是四个 workload 共用的命令行参数。每个 workload 有自己的 adapter。

| matrix id | `logical_threads=8` 实际变成 |
| --- | --- |
| `alloc-test` | `alloc-test 8` |
| `larson` | `larson 2.5 7 8 1000 10000 42 8`。只有最后一个数跟着变。论文那一档是 `42 100`，不在这 96 组里 |
| `xmalloc` | `xmalloc-test -w 8 -t 5 -s -1`。`-w` 是 8 个生产者，二进制再起 8 个消费者，一共 16 个 OS 线程。2019 年脚本里的 `-w $((2*procs))` 记在结果里，但 sweep 不用它 |
| `redis` | server 和 client 的参数都不变。`-P 8` 是 pipeline，不是线程列。六行矩阵是同一条命令，不是扩展曲线 |

Redis 的 `measurement_subject` 是 server。perf 和 RSS 挂在 server pid 上。不允许 `perf stat redis-benchmark`。

别名是显式表，不靠字符串猜测。`xmalloc-test` → `xmalloc`，`mi` → `mimalloc-v1.0.0`，`je` → `jemalloc`，`tc` → `tcmalloc`。其余拼写直接报错。

resume 的单位是一次 invocation：`workload / allocator / logical_threads / phase / repetition`。先写临时文件再 `rename`。`exit_code=0`、metric parser 成功、allocator 校验成功，三条都成立才算 complete。文件里自己写 `complete: true` 不算数。

统计量不合并。验收用 `protocol_statistics`：15 个 measured sample 的 median 和 p95，warmup 不算。论文的 5 次平均是另一个函数 `paper_view`，不能写进验收字段。

同一 `(workload, logical_threads)` 块里，allocator 和 repetition 用固定种子 `20190621` 打散。块的顺序仍按矩阵。换种子只改顺序，不改判据。

```bash
cd papers/mimalloc-2019/bench
python3 -m runner preflight --strict   # 硬件线程不足时退出码 2
python3 -m runner dry-run --profile round1 --out runs/dry-round1
python3 -m runner dry-run --profile smoke --out runs/dry-smoke
python3 -m unittest runner.tests.test_runner
```

`runs/` 不入库。dry-run 写出 `host.json`、`plan.json`、`artifacts.json`。二进制和 `.so` 现在都是 `unresolved`。正式 1728 次还不在这个包里。

round-1 至少要 12 个硬件线程。不够的机器可以 dry-run，preflight 会把 `eligible_for_round_1` 标成 false。
