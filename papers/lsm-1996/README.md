# Log-Structured Merge-Tree (LSM-tree)

Patrick O'Neil et al. *The Log-Structured Merge-Tree (LSM-Tree)*. Acta Informatica 1996.

Modern realizations: LevelDB, RocksDB.

- Original: https://www.cs.umb.edu/~poneil/lsmtree.pdf
- LevelDB: https://github.com/google/leveldb

**Status**: architecture-extraction (learning-oriented).

## Why this paper

引入存储引擎架构原语：

```
WAL
 ↓
MemTable (mutable)
 ↓
Immutable MemTable
 ↓
SSTable (L0)
 ↓
Leveled Compaction (L1..Ln)
```

与现有篇目完全不同：不是 runtime / VM / concurrency，而是写优化的多层不可变存储。

## 学习目标

- 写路径：append WAL + 插入有序 MemTable
- Flush：MemTable → immutable → SSTable
- Compaction：多路归并，保持层内 key range 不重叠（L1+）
- 读路径：最新优先 + Bloom / index

## 第一轮

- `notes/architecture.md`
- 极简 model：内存 MemTable + 模拟 SSTable + 简单 compaction
- 探针：写入、flush、读取正确性
