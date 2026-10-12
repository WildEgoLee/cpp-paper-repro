# Copy-on-Write (CoW)

Delayed copy via shared read-only + write-time clone.

Classic references:
- Unix `fork` (1970s)
- ZFS / Btrfs CoW filesystems
- APFS clones

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入延迟复制原语：

```
share(page)
  → both holders mark read-only, refcount++
write(page)
  → if refcount > 1: allocate new, copy, point to new, refcount--
  → if refcount == 1: just make writable
```

介于不可变结构（永远复制路径）和可变结构（立即写）之间。

## 学习目标

- 引用计数与共享
- 写时才复制
- 单一所有者优化（refcount==1 不复制）
- 与 fork / 文件系统快照的关系

## 第一轮

- `notes/architecture.md`
- 极简 model：带引用计数的页
- 探针：共享后写一次，两边数据不同
