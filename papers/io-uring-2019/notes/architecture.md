# io_uring Architecture Notes

学习目标：用户态与内核态通过共享环形队列做异步 I/O。

## 核心对象

```
SubmissionQueue (SQ)   // user → kernel
  SQE                  // 请求描述（opcode, fd, addr, len, user_data）
CompletionQueue (CQ)   // kernel → user
  CQE                  // 结果（user_data, res, flags）
Ring                   // head / tail + mask
```

## Ownership

- SQ 由用户生产，内核消费。
- CQ 由内核生产，用户消费。
- 共享内存由 mmap 建立；双方只通过 head/tail 协调。

## Fast path / Slow path

- Fast：用户填 SQE 并推进 SQ tail（无 syscall）。
- Slow：`io_uring_enter` 通知内核，或等待 CQ。

## Invariants

1. 每个提交的 SQE 最终对应一个 CQE（普通模式）。
2. head/tail 只有生产者推进 tail，消费者推进 head。
3. 环形缓冲区用 mask 取模，不移动数据。

## Policy / Mechanism 分离

- Policy：何时提交、是否批量、是否轮询。
- Mechanism：环形队列、mmap、内核消费 SQE 并产生 CQE。

## Concurrency boundary

- 单线程拥有一对 SQ/CQ 时几乎无锁（SPSC）。
- 多线程提交需要同步或每线程一个 ring。

## State 位置

- 待处理请求：SQ 环。
- 已完成结果：CQ 环。
- 用户关联：SQE.user_data。

## Data layout 理由

共享内存避免每次 I/O 都 syscall + 拷贝；环形结构让生产者和消费者只动指针。

## Failure modes

- CQ 溢出 → 丢弃或进入慢路径。
- 用户在完成前释放 buffer → 数据损坏。
- 未调用 enter 而内核未轮询 → 请求挂起。

## 与传统方式对比

| | 提交开销 | 完成通知 | 批量 |
|--|----------|----------|------|
| read/write | syscall | 同步 | 无 |
| epoll | syscall + 事件 | 事件驱动 | 有 |
| io_uring | 共享环 | 共享环 | 强 |
