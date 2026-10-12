# Fibers Architecture Notes

学习目标：用户态协作式多任务。

## 核心对象

```
Fiber
  stack          // 独立栈
  context        // 保存的寄存器 / IP / SP
  state          // runnable / suspended / done
Scheduler        // 选择下一个 fiber 并 switch
```

## Ownership

- 每个 Fiber 拥有自己的栈和上下文。
- Scheduler 拥有 ready 队列。
- 同一时刻只有一个 fiber 在某个 OS 线程上运行。

## Fast path / Slow path

- Fast：yield 时只切换寄存器和栈指针（无 syscall）。
- Slow：真正阻塞 I/O 时需要把 fiber 挂起，让 OS 线程去做其他事。

## Invariants

1. 只有显式 yield 才会切换。
2. 同一 OS 线程上的 fiber 之间无数据竞争（串行）。
3. 切换后必须恢复完整上下文。

## Policy / Mechanism 分离

- Policy：调度策略（轮转、优先级、work-stealing）。
- Mechanism：栈切换、上下文保存/恢复。

## Concurrency boundary

- 同一 OS 线程上的 fiber：协作式，无锁。
- 跨 OS 线程：仍需同步（或 fiber migration）。

## State 位置

- 运行时状态：fiber 的栈 + 寄存器。
- 就绪队列：scheduler。

## Data layout 理由

独立栈让 fiber 可以在任意嵌套调用处 yield（stackful）。
栈大小可配置，通常比 OS 线程栈小。

## Failure modes

- 忘记 yield → 其他 fiber 饥死。
- 栈溢出。
- 在持有锁时 yield → 死锁（若锁不是 fiber-aware）。

## 与其他模型对比

| | 调度 | 切换开销 | 并行 |
|--|------|----------|------|
| OS thread | 预抢占 | 高 | 真并行 |
| Fiber | 协作 | 低 | 同线程串行 |
| Actor | 消息 | 低 | 可并行 |
