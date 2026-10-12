# LSM-tree Architecture Notes

学习目标：写优化存储引擎的多层不可变设计。

## 核心对象

```
MemTable          // 可变有序内存表（skip list / map）
ImmutableMemTable // 冻结的 MemTable，等待 flush
SSTable           // 不可变有序文件（带稀疏 index + Bloom）
Level             // 一组 SSTable，L0 可重叠，L1+ 不重叠
Compaction        // 多路归并，产生下一层
WAL               // 持久性日志
```

## Ownership

- DB 拥有当前 MemTable、immutable 列表、各层 SSTable。
- SSTable 一旦写出即不可变；旧文件由 compaction 决定删除。
- 读者看到的是一个一致的 snapshot（某个时刻的文件集合）。

## Fast path / Slow path

- Fast write：WAL append + MemTable insert。
- Slow write：MemTable 满 → freeze → 后台 flush 成 L0 SSTable。
- Fast read：MemTable 命中。
- Slow read：从新到旧查 SSTable（Bloom 过滤 + index）。

## Invariants

1. MemTable 内 key 有序。
2. SSTable 不可变且内部有序。
3. L1 及以下同一层内 SSTable key range 不重叠。
4. 同一 key 的最新版本在更高层（或 MemTable）。

## Policy / Mechanism 分离

- Policy：何时 flush、选哪个文件做 compaction、level size ratio。
- Mechanism：多路归并、写新 SSTable、更新 manifest。

## Concurrency boundary

- 前台写入 MemTable（通常加锁或无锁 skip list）。
- 后台 compaction 线程读旧文件、写新文件。
- 读者可与 compaction 并发（旧文件仍可读直到被删除）。

## State 位置

- 活跃数据：MemTable。
- 持久数据：SSTable 文件 + WAL。
- 元数据：manifest（哪亚文件属于哪一层）。

## Data layout 理由

把随机写转成顺序写（WAL + sorted flush），用 compaction 控制读放大和空间放大。

## Failure modes

- 崩溃未 flush → 依靠 WAL 重放。
- Compaction 失败 → 旧文件仍然有效，新文件丢弃。
- L0 文件过多 → 读放大严重。

## 与 B-tree 对比

| | 写 | 读 | 空间 |
|--|----|----|------|
| B-tree | 随机 I/O | 优秀 | 低放大 |
| LSM | 顺序优秀 | 多层合并 | 可控制 |
