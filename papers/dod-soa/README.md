# Data-Oriented Design (SoA)

Layout primitive: **Structure of Arrays** vs Array of Structures.

Classic references:
- Richard Fabian, *Data-Oriented Design*
- Mike Acton, “Data-Oriented Design and C++” (CppCon)
- ECS / archetype storage in game engines

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入数据布局原语：

```
AoS:  [x y z w][x y z w][x y z w] ...
SoA:  x[]  y[]  z[]  w[]
```

当热路径只访问少数字段时，SoA 让缓存行被充分利用，并便于 SIMD。

与前面的并发、存储、调度完全不同。

## 学习目标

- AoS vs SoA 的缓存行利用率
- 何时选 SoA（批量访问少数字段）
- 何时选 AoS（随机访问完整对象）
- 简单性能对比

## 第一轮

- `notes/architecture.md`
- 极简 model：相同计算的 AoS 与 SoA 版本
- 探针：比较两者在“只读 x”路径上的时间
