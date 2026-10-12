# Raft Consensus

Diego Ongaro, John Ousterhout. *In Search of an Understandable Consensus Algorithm*. USENIX ATC 2014.

- Paper: https://raft.github.io/raft.pdf

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入分布式共识原语：

```
Term
  ├─ Leader election  (RequestVote, majority)
  └─ Log replication   (AppendEntries, commit on majority)

States: Follower ↔ Candidate → Leader
```

与前面所有单机系统都不同。

## 学习目标

- Term 作为逻辑时钟
- Leader election（多数派）
- Log replication（从 leader 到 follower）
- Commit 要求多数派

## 第一轮

- `notes/architecture.md`
- 极简 model：3 节点、term、投票、log
- 探针：选举 + 复制一条日志并提交
