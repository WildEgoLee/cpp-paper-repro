# Actor Model Architecture Notes

学习目标：消息传递并发，无共享可变状态。

## 核心对象

```
Actor
  mailbox     // 异步消息队列
  state       // 私有，只有自己可改
  behavior    // 处理下一条消息的逻辑
Address       // 不可伪造的 actor 引用
```

## Ownership

- 每个 Actor 拥有自己的 state 和 mailbox。
- 消息被发送后所有权转移给接收方。
- 没有共享可变数据。

## Fast path / Slow path

- Fast：本地 mailbox 入队。
- Slow：跨线程/跨进程投递、调度 actor 到线程。

## Invariants

1. 只有拥有 address 才能发消息。
2. Actor 的 state 只在处理自己消息时被修改。
3. 消息传递是异步的，不保证到达顺序。

## Policy / Mechanism 分离

- Policy：如何调度 actor（每 actor 一线程、线程池、work-stealing）。
- Mechanism：mailbox、消息投递、become。

## Concurrency boundary

- Actor 之间：只有消息。
- Actor 内部：串行处理自己的 mailbox（无锁）。

## State 位置

- 私有状态：actor 内部。
- 待处理消息：mailbox。

## Data layout 理由

Mailbox 通常是队列；state 可以是任意结构，因为只有单线程访问。

## Failure modes

- 消息丢失（无可靠传递保证）。
- Mailbox 无界增长 → 内存耗尽。
- 死锁：循环等待（需监控或超时）。

## 三个原语

| 原语 | 作用 |
|------|------|
| create | 新建 actor，返回 address |
| send | 异步发消息到 address |
| become | 替换自己的 behavior |
