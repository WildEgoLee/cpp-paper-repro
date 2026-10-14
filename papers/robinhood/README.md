# Robin Hood Hashing

Pedro Celis, Per-Ake Larson, J. Ian Munro. *Robin Hood Hashing*. FOCS 1985.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入有界方差探测原语：

```
Probe Sequence Length (PSL) = distance from home slot

insert(x):
  walk the probe sequence
  if an occupied slot has smaller PSL than x would have,
    steal it: place x, continue with the displaced key
```

富者被偷，贫者得到更近的槽。探测长度方差有界。与 Cuckoo 的双表踢出完全不同。

## 学习目标

- PSL（探测序列长度）
- 富者被偷
- 为什么方差有界
- 与普通线性探测的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：线性探测 + PSL 比较
- 探针：插入后查找，富者被偷
