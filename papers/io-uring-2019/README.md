# io_uring

Jens Axboe. *Efficient IO with io_uring*. 2019.

Linux asynchronous I/O interface.

- Design note: https://kernel.dk/io_uring.pdf
- Kernel: linux/io_uring
- Userspace: liburing

**Status**: architecture-extraction (learning-oriented).

## Why this paper

引入异步 I/O 架构原语：

```
User                          Kernel
  |                              |
  |  SQ (submission queue)       |
  |  --------------------------> |
  |                              |
  |  CQ (completion queue)       |
  |  <-------------------------- |
  |                              |
  shared memory rings (mmap)
```

与现有篇目完全不同：不是调度、存储引擎或并发原语，而是用户态/内核态共享环形队列的零拷贝提交。

## 学习目标

- SQE / CQE 结构
- head/tail + ring mask（无锁 SPSC）
- mmap 共享内存
- 批量提交 + 完成通知

## 第一轮

- `notes/architecture.md`
- 极简 model：用户态模拟 SQ/CQ 环（无真实内核）
- 探针：提交若干 SQE，模拟完成后从 CQ 取回
