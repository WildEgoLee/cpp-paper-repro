# Fibers

User-level cooperative multitasking (stackful coroutines).

Classic references:
- Boost.Fiber
- Windows Fibers (`CreateFiber` / `SwitchToFiber`)
- `ucontext` / `swapcontext`

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入协作式用户态多任务原语：

```
Fiber
  ├─ own stack
  ├─ saved registers / instruction pointer
  └─ yield() → switch to another fiber
```

无内核预抢占。与 OS 线程、work-stealing、Actor 都不同。

## 学习目标

- 协作式 vs 预抢占式
- 栈切换（用户态）
- 同一 OS 线程上跑多个 fiber
- yield 点是唯一的调度点

## 第一轮

- `notes/architecture.md`
- 极简 model：显式 yield + 简单调度器（先不做真实栈切换）
- 探针：多 fiber 互相 yield 并正确交替执行
