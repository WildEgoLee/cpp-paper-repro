# Mesh Architecture Notes

学习目标：从「会写 C++」进入「理解 C++ runtime 与 OS 虚拟内存边界」。

## 核心对象

```
logical object
    ↓
virtual address (stable)
    ↓
span (same-size objects)
    ↓
physical backing (file-backed page)
```

## Ownership

- Arena 拥有 file descriptor 和虚拟地址空间。
- MiniHeap 拥有一个 span 的 bitmap 和元数据。
- 对象本身不拥有内存；地址是稳定的。

## Fast path / Slow path

- Fast: thread-local shuffle vector 分配（O(1) 随机 offset）。
- Slow: 全局锁 + SplitMesher 搜索 + consume + remap + scavenge。

## Invariants

1. 对象虚拟地址永不改变。
2. 两个 span 可 mesh 当且仅当 live bitmap 不重叠。
3. remap 后两个 VA 看到相同物理内容。
4. punch hole 后原物理页归还 OS。

## Policy / Mechanism 分离

- Policy: 哪亚vailable个 span 可 mesh（随机探测、occupancy 阈值）。
- Mechanism: copy + mmap(MAP_FIXED) + fallocate(PUNCH_HOLE)。

## Concurrency boundary

- beginMesh: mprotect(src, PROT_READ) 防止复制期间写。
- 全局锁保护 meshing；快路径无锁。

## State 位置

- Bitmap 在 MiniHeap。
- 物理页在 memfd 文件。
- 虚拟映射在进程页表。

## Data layout 理由

随机分配使 non-overlap 成为高概率事件，从而允许不移动对象的压缩。

## Failure modes

- 所有 span 在相同 offset 都有对象 → 无法 mesh。
- 并发写源 span 时未 mprotect → 数据损坏。
- punch 失败 → 物理内存泄漏。

## 与上游差异

探针已覆盖 L0–L4（bitmap、搜索、span、真实 mmap）。
缺少完整 MiniHeap 元数据、shuffle vector、后台线程、多 size class。
