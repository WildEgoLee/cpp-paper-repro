# SIMD Architecture Notes

学习目标：数据并行。

## 核心对象

```
Vector register   // 128/256/512-bit，装多个元素
SIMD instruction  // 一条指令处理整个向量
Lane              // 向量中的一个元素
```

## Ownership

- 向量寄存器拥有当前指令的操作数。
- 内存布局（SoA）决定能否一次 load 整个向量。

## Fast path / Slow path

- Fast：连续 SoA 数据，直接 SIMD load + 运算。
- Slow：分支、gather、不对齐 → 退回标量。

## Invariants

1. 同一指令对所有 lane 执行相同操作。
2. 数据必须连续且同质才能充分利用。
3. 宽度由 ISA 决定（SSE 4×f32，AVX 8×f32）。

## Policy / Mechanism 分离

- Policy：哪些循环值得向量化。
- Mechanism：向量寄存器、SIMD 指令、内存对齐。

## Concurrency boundary

SIMD 是单线程内的数据并行；与多核正交。

## State 位置

- 操作数：向量寄存器。
- 数据：内存中的连续数组。

## Data layout 理由

SoA 让同一字段连续，一次 SIMD load 就能装满寄存器。
AoS 需要 gather，慢。

## Failure modes

- 数据依赖分支 → 向量化失败。
- 不对齐 → 慢速路径。
- 环路循环依赖 → 无法并行。

## 与其他并行对比

| | 并行度 | 开销 |
|--|--------|------|
| 多线程 | 任务 | 上下文切换 |
| SIMD | 数据 | 几乎零 |
| GPU | 数据 | 传输 |
