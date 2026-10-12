# Software Transactional Memory (STM)

Optimistic concurrency via transactions.

Classic references:
- Herlihy & Moss, *Transactional Memory: Architectural Support for Lock-Free Data Structures* (ISCA 1993)
- Shavit & Touitou, *Software Transactional Memory* (PODC 1995)
- Dice, Shalev, Shavit, *Transactional Locking II* (DISC 2006)

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入乐观并发原语：

```
transaction {
  read set   // 记录读过的地址 + 版本
  write set  // 缓冲写
  commit     // 验证读集，无冲突则提交
  abort      // 冲突则回滚
}
```

与锁、RCU、无锁结构都不同。

## 学习目标

- Read-set / write-set
- 乐观读、悲观写（或反之）
- Commit 时冲突检测
- Abort 与重试

## 第一轮

- `notes/architecture.md`
- 极简 model：版本号 + 读集/写集
- 探针：两个事务冲突时一个 abort
