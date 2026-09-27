# runner

P1.1 的执行契约。仍然只有 preflight 和 dry-run，没有 execute，也没有 benchmark 数字。

冻结矩阵没改。`logical_threads` 是矩阵列，不是 OS 线程数。

| 矩阵列 = 12 | 实际并发 | 扩展性图用哪一列 |
| --- | --- | --- |
| alloc-test、larson | 12 个 worker | 矩阵列。12 个硬件线程的门槛只保护这两项 |
| xmalloc | 12 个生产者 + 12 个消费者 = 24 | `effective_worker_threads`。在 12 硬件线程上 `oversubscribed=true`。门槛不因此改成 24 |
| redis | 命令不随矩阵列变化 | 不是 scaling row。`oversubscribed` 是 null |

Redis 的 1/2/4/6/8/12 有六个 `resolved_configuration_id`，但同一个 `execution_equivalence_key`。1728 次都还在。C1 和扩展性图不能把这六行当成线程扩展。

每个 metric 自己带 subject 和 window：

| workload | wall / throughput | perf | peak RSS |
| --- | --- | --- | --- |
| foreground | 进程 lifetime | 进程 lifetime。`perf stat` 从 exec 包住进程，不允许先启动再 attach | 进程 lifetime |
| redis | client 的 request-window | server 的同一段 request-window | server 的 lifetime，不是 client |

`request-window` 不含 server 启动和 shutdown。进程在校验 `/proc/maps` 之前已经退出，这次 invocation 不算 complete。

round-1 的 12 硬件线程门槛还在，作用范围写在 `host.json` 的 `gate_scope`。xmalloc 的超订按 invocation 记，不抬高这个门槛。

```bash
cd papers/mimalloc-2019/bench
python3 -m runner preflight --strict
python3 -m runner dry-run --profile round1 --out runs/dry-round1
python3 -m unittest runner.tests.test_runner
```

`runs/` 不入库。正式执行是下一笔，不在这个提交里。
