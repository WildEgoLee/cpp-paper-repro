# STM Architecture Notes

学习目标：乐观并发控制。

## 核心对象

```
Transaction
  read_set    // addr → version observed
  write_set   // addr → new value (buffered)
  status      // active / committed / aborted
Versioned location
  value + version
```

## Ownership

- 每个事务拥有自己的 read/write set。
- 共享数据带版本号。
- Commit 成功后写集才变得对他人可见。

## Fast path / Slow path

- Fast：无冲突时直接 commit。
- Slow：检测到冲突 → abort + 重试。

## Invariants

1. 事务的写对外界不可见，直到 commit。
2. Commit 时读集中所有地址的版本必须仍与读取时一致。
3. Abort 丢弃写集，不改变共享状态。

## Policy / Mechanism 分离

- Policy：冲突时谁 abort（重试、优先级、回退）。
- Mechanism：版本检测、写缓冲、commit 协议。

## Concurrency boundary

- 事务内部：串行。
- 事务之间：通过读/写集交集检测冲突。

## State 位置

- 暂存写：事务私有 write set。
- 已提交数据：共享内存 + 版本号。

## Data layout 理由

版本号让读可以乐观进行；写缓冲让 abort 便宜。

## Failure modes

- 读集版本不匹配 → abort。
- 写写冲突 → 至少一个 abort。
- 长事务容易与短事务冲突（饥死）。

## 与其他模型对比

| | 同步 | 冲突检测 | 回滚 |
|--|------|----------|------|
| Mutex | 悲观 | 锁 | 无 |
| RCU | 读无锁 | 宽限期 | 无 |
| STM | 乐观 | 读/写集 | 有 |
