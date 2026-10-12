# Michael-Scott Lock-Free Queue

Maged M. Michael, Michael L. Scott. *Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms*. PODC 1996.

- Paper: https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf

**Status**: architecture-extraction (learning-oriented).

## Why this paper

引入无锁队列原语：

```
Node { value, next }
head → dummy → ... → tail

enqueue: CAS(tail->next, null, new) then swing tail
dequeue: CAS(head, head, head->next)
```

核心技巧：永久 dummy 节点 + 两步 CAS。
与 RCU（宽限期）、work-stealing（deque）不同。

## 学习目标

- Dummy sentinel 避免空队列特殊情况
- Enqueue 的两步（link + swing tail）
- ABA 问题与解决（计数器 / hazard pointer / RCU）
- 内存序（acquire/release）

## 第一轮

- `notes/architecture.md`
- 极简 model：CAS 队列接口
- 探针：多线程 enqueue/dequeue 正确性
