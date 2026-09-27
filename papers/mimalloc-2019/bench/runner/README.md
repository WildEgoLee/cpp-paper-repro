# runner

P1.2 executor。没有真实 allocator 的 benchmark 数字。smoke 不在这台机器上跑。

`execution_equivalence_key` 只给分析分组。`deduplicate_execution` 是 false。round-1 仍然调度 1728 条 invocation。resume 只认 `invocation_id`。Redis 的六行不能被合并掉。

执行前会把 `{artifact:*}` / `{lib:*}` 解析成绝对路径并写入 SHA-256。glibc 记的是本机 `libc.so.6` 和 loader，不是一个 preload 文件。resume 时重新哈希；对不上就拒绝。run 一旦 seal，dry-run 不能覆盖 `plan.json`。

子进程环境会先去掉调用者的 `LD_PRELOAD`，再只放计划里的那一个。foreground 在 `exec` 之前停住，父进程拿到 pid、装好测量，再放行。`/proc/<pid>/maps` 要对解析后的真实路径，basename 不算命中。peak RSS 来自这个 child 的 `wait4` `ru_maxrss`（KiB）。`perf stat -p` 不带 `--no-inherit`。

perf 缺失或 `<not supported>` 是单个计数器的 `unavailable` / `unsupported`，case 仍可以 complete。输出解析不了才是 `parsed: false`，raw 文件留下，下一次 resume 写新的 attempt，不删旧的。

Redis 的 server 是 runner 的直接 child。PING 之后查 maps，再对 server 打开 request-window 的 perf，然后才跑 client。client 成功但 server 失败，不算 complete。client 不继承 `LD_PRELOAD`。

```bash
cd papers/mimalloc-2019/bench
python3 -m unittest runner.tests.test_runner runner.tests.test_execute
python3 -m runner execute --run runs/some-sealed-dir
```

`runs/` 不入库。32 次真实 smoke 等至少 12 个硬件线程的 Linux。
