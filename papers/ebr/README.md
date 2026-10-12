# Epoch-based Reclamation (EBR)

Safe memory reclamation via global epochs.

Classic references:
- Keir Fraser, *Practical Lock-Freedom* (PhD thesis, 2004)
- Hart et al., *Performance of Memory Reclamation for Lockless Synchronization* (JPDC 2007)

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入基于 epoch 的宽限期回收：

```
global_epoch
  ├─ thread announces local_epoch on enter
  ├─ retire(node) → limbo list of current epoch
  └─ when all threads have observed epoch e
       → free limbo list of epoch e-2
```

与 hazard pointers（每指针保护）、RCU（宽限期）都不同。

## 学习目标

- 全局 epoch + 线程宣告
- 为什么要等两个 epoch
- Limbo list
- 与 hazard pointer 的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：3 epoch 轮转 + 宣告
- 探针：retire 后推进 epoch，旧节点被释放
