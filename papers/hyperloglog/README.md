# HyperLogLog

Philippe Flajolet, Éric Fusy, Olivier Gandouet, Frédéric Meunier. *HyperLogLog: the analysis of a near-optimal cardinality estimation algorithm*. AOFA 2007.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入基数估计原语：

```
hash(x) → index (first b bits) + rest
registers[index] = max(registers[index], leading_zeros(rest))

estimate = alpha * m^2 / sum(2^-registers[i])
```

用调和平均把多个前导零观察合成基数估计。空间与元素数无关。与成员测试完全不同。

## 学习目标

- 前导零与基数的关系
- 随机平均（多个寄存器）
- 调和平均为什么抗异常值
- 与精确计数的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：多寄存器 + 调和平均
- 探针：插入后估计，与真实基数接近
