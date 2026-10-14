# B+ Tree

Ordered index with leaf-level linked list.

Classic references:
- Bayer & McCreight, *Organization and Maintenance of Large Ordered Indexes* (1972)
- Comer, *The Ubiquitous B-Tree* (1979)

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入有序索引原语：

```
Internal nodes: separator keys + child pointers only
Leaf nodes:    all data + next-leaf pointer
Range scan:    one root-to-leaf + sequential leaf walk
```

与 LSM 的只追加设计完全不同。叶子链表让范围查询变成 O(log N + K)。

## 学习目标

- 内部节点 vs 叶子节点分离
- 叶子链表
- 页分裂（copy-up vs push-up）
- 范围扫描

## 第一轮

- `notes/architecture.md`
- 极简 model：叶子链表 + 插入
- 探针：插入几个键后范围扫描
