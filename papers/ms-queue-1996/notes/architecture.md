# Michael-Scott Queue Architecture Notes

学习目标：无锁 FIFO 队列的经典设计。

## 核心对象

```
Node { T value; atomic<Node*> next; }
Queue { atomic<Node*> head; atomic<Node*> tail; }
```

永久存在一个 dummy 节点，head 和 tail 初始都指向它。

## Ownership

- 节点在被 dequeue 之前属于队列。
- Dequeue 后节点可被回收（需处理 ABA）。
- Dummy 永不被删除。

## Fast path / Slow path

- Fast enqueue：CAS 成功把新节点链到 tail->next，再摆动 tail。
- Slow：tail 滞后时帮助摆动，或 CAS 失败重试。

## Invariants

1. 队列永远至少有 dummy 节点。
2. head 指向的是已被「逻辑删除」的节点（真正的队头是 head->next）。
3. 所有可达节点的 next 链是一致的。

## Policy / Mechanism 分离

- Policy：如何回收旧节点（计数器、hazard pointer、RCU、epoch）。
- Mechanism：CAS 链接 + 摆动指针。

## Concurrency boundary

- Enqueue 与 dequeue 可并发。
- 多个 enqueue 通过 CAS 竞争 tail->next。
- 多个 dequeue 通过 CAS 竞争 head。

## State 位置

- 逻辑队列：从 head->next 到 tail。
- 物理节点：包含 dummy。

## Data layout 理由

Dummy 让 head/tail 永不为 null，消除空队列分支，使并发逻辑更简单。

## Failure modes

- ABA：节点被重用后 CAS 错误成功。
- 内存序错误 → 看到半初始化节点。
- 未摆动 tail → 后续 enqueue 变慢（但仍正确）。

## 与其他技术对比

| | 同步 | 空队列处理 | ABA |
|--|------|------------|-----|
| Mutex queue | 锁 | 简单 | 无 |
| MS queue | CAS | dummy | 需处理 |
| Array ring | 索引 CAS | 模运算 | 较少 |
