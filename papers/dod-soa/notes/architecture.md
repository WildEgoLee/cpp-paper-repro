# Data-Oriented Design Architecture Notes

学习目标：数据布局如何影响缓存与向量化。

## 核心对象

```
ParticleAoS { float x, y, z, w; }   // 交错
ParticleSoA { float* x; float* y; float* z; float* w; }  // 按字段分离
```

## Ownership

- AoS：每个对象拥有自己的字段。
- SoA：数组拥有某一字段的所有实例。

## Fast path / Slow path

- Fast（SoA）：热路径只触及少数字段，缓存行几乎全是有用数据。
- Slow（AoS）：同一路径下缓存行被无用字段占满。

## Invariants

1. 同一索引在所有 SoA 数组中表示同一实体。
2. 布局选择必须与主要访问模式匹配。

## Policy / Mechanism 分离

- Policy：哪些字段是热的（放 SoA），哪些是冷的（可以 AoS）。
- Mechanism：数组存储与遍历。

## Concurrency boundary

SoA 使同一字段的并行更新更容易（无跨字段写冲突）。

## State 位置

- AoS：对象内部。
- SoA：按字段分布在独立数组。

## Data layout 理由

CPU 缓存行通常 64 B。
只读一个 float 时：
- AoS 大量浪费带宽
- SoA 几乎满载有用数据，且可直接 SIMD load

## Failure modes

- 错误布局 → 缓存命中率低、预取失效。
- SoA 下随机访问完整对象 → 多次缓存未命中。
- 索引不同步 → 实体错乱。

## 对比

| 访问模式 | AoS | SoA |
|----------|-----|-----|
| 批量读单一字段 | 差 | 优 |
| 随机读完整对象 | 优 | 差 |
| SIMD | gather | 直接 load |
