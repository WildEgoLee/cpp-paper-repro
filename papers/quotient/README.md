# Quotient Filter

Michael A. Bender, Martin Farach-Colton, Rob Johnson, Russell Kraner, Bradley C. Kuszmaul, Dzejla Medjedovic, Pablo Montes, Pradeep Shetty, Richard P. Spillane, Erez Zadok. *Don't Thrash: How to Cache Your Hash on Flash*. SEA 2012.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入商与余数原语：

```
hash(x) = quotient || remainder
slots[quotient] stores remainder
run = contiguous remainders with same quotient

lookup: scan the run for the remainder
delete: remove the remainder from the run
```

缓存局部性好。支持删除。与 Cuckoo Filter 的指纹完全不同。

## 学习目标

- 商与余数
- 运行长度
- 为什么局部性好
- 与 Cuckoo Filter 的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：商与余数 + 运行
- 探针：插入后查找，删除后不在
