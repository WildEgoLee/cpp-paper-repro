# Space-Saving

Ahmed Metwally, Divyakant Agrawal, Amr El Abbadi. *Efficient Computation of Frequent and Top-k Elements in Data Streams*. ICDT 2005.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入重击者追踪原语：

```
counters[k]   // 固定 k 个 (item, count)

add(x):
  if x is tracked: ++count
  else: replace the minimum-count item with x, count = min + 1
```

最小计数被淘汰。估计误差有界。与 CMS 的过估计完全不同。

## 学习目标

- 计数器替换
- 为什么最小计数被淘汰
- 重击者不会被淘汰
- 与 CMS 的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：固定 k 个计数器 + 替换最小
- 探针：插入后查找，重击者仍在
