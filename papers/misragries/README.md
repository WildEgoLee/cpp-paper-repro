# Misra-Gries

Jayadev Misra, David Gries. *Finding Repeated Elements*. Science of Computer Programming 1982.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入频繁项原语：

```
counters[k-1]

add(x):
  if x is tracked: ++count
  else if free slot: start at 1
  else: decrement ALL counters; drop zeros
```

全体减一。频率 > n/k 的元素不会被丢弃。与 Space-Saving 的替换完全不同。

## 学习目标

- 全体减一
- 为什么频繁项会留下
- 与 Space-Saving 的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：固定计数器 + 全体减一
- 探针：插入后查找，频繁项仍在
