# Cuckoo Hashing

Rasmus Pagh and Flemming Friche Rodler. *Cuckoo Hashing*. ESA 2001.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入踢出链原语：

```
Two tables T1, T2
Two hash functions h1, h2

lookup(x):  check T1[h1(x)] or T2[h2(x)]   // worst-case O(1)
insert(x):  place at h1(x); if occupied, kick the old key
            to its other location; repeat until empty slot
```

最坏情况查找是常数。与 Bloom 的概率成员测试完全不同。

## 学习目标

- 双表定位
- 踢出链
- 为什么查找是最坏情况 O(1)
- 循环时需要 rehash

## 第一轮

- `notes/architecture.md`
- 极简 model：双表 + 踢出
- 探针：插入后查找，踢出发生
