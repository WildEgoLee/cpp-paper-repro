# SIMD / Vectorization

Single Instruction, Multiple Data.

Classic references:
- Flynn's taxonomy (1966)
- Intel SSE / AVX / AVX-512
- ARM NEON / SVE

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入数据并行原语：

```
scalar:  for i: c[i] = a[i] + b[i]     // 1 add per instruction
SIMD:    c[i:i+8] = a[i:i+8] + b[i:i+8]  // 8 adds per instruction
```

与线程并行、消息传递、共识都不同。与 SoA 布局配合。

## 学习目标

- 数据并行 vs 任务并行
- 向量寄存器与宽度
- SoA 使 SIMD load 变得可能
- 何时向量化失败（分支、gather）

## 第一轮

- `notes/architecture.md`
- 极简 model：标量 vs 手动展开的加法
- 探针：比较两者时间
