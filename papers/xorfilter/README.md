# Xor Filter

Thomas Mueller Graf, Daniel Lemire. *Xor Filters: Faster and Smaller Than Bloom and Cuckoo Filters*. Journal of Experimental Algorithmics 2020.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入完美哈希与 XOR 原语：

```
table[c]
contains(x) = (table[h0(x)] XOR table[h1(x)] XOR table[h2(x)]) == fingerprint(x)
```

静态集合。空间更小。与动态过滤器完全不同。

## 学习目标

- 完美哈希
- XOR 映射
- 为什么静态
- 与 Bloom/Cuckoo 的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：三哈希 XOR
- 探针：构建后查找，已插入的在
