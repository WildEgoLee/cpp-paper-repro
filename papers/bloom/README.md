# Bloom Filter

Burton H. Bloom. *Space/Time Trade-offs in Hash Coding with Allowable Errors*. CACM 1970.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入概率成员测试原语：

```
bit array[m]
k independent hash functions

insert(x):  set bit[h_i(x)] = 1 for i in 1..k
query(x):   if any bit[h_i(x)] == 0 → definitely absent
            if all 1 → probably present (false positive possible)
```

无假阴性。空间与元素数无关，只与假阳性率有关。

## 学习目标

- 位数组 + 多哈希
- 为什么不可能假阴性
- 假阳性率与 m/n/k 的关系
- 空间换准确率

## 第一轮

- `notes/architecture.md`
- 极简 model：位数组 + 2 个哈希
- 探针：插入后查询，未插入的可能假阳性
