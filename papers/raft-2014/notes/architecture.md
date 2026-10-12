# Raft Architecture Notes

学习目标：分布式共识的解耦。

## 核心对象

```
Server
  state          // Follower / Candidate / Leader
  currentTerm
  votedFor
  log[]          // {term, command}
  commitIndex
Cluster
  majority       // N/2 + 1
```

## Ownership

- 每个 server 拥有自己的 log 和 term。
- Leader 拥有复制的责任；Follower 被动。
- 已提交的日志条目不会被覆盖。

## Fast path / Slow path

- Fast：Leader 存活时 AppendEntries 复制。
- Slow：Leader 失败 → 选举。

## Invariants

1. 每个 term 至多一个 leader。
2. 日志只能追加，不能修改已有条目。
3. 已提交的条目一定出现在未来 leader 的日志中。

## Policy / Mechanism 分离

- Policy：选举超时随机化、心跳间隔。
- Mechanism：RequestVote、AppendEntries、majority。

## Concurrency boundary

- 节点之间：RPC。
- 节点内部：串行处理消息。

## State 位置

- 持久：currentTerm、votedFor、log。
- 易失：当前 leader、超时。

## Data layout 理由

Log 是有序数组；条目带 term 以便检测不一致。

## Failure modes

- 分裂投票 → 选举失败，下一 term 重试。
- 网络分区 → 少数派不能提交。
- 旧 leader 回来 → 被更高 term 拒绝。

## 三个子问题

| 子问题 | 机制 |
|--------|------|
| Leader election | RequestVote + majority |
| Log replication | AppendEntries |
| Safety | 日志足够新才能当选 |
