# Generational Garbage Collection

Nursery + old generation + write barrier.

Classic references:
- Ungar, *Generation Scavenging* (1984)
- Lieberman & Hewitt, *A Real-Time Garbage Collector Based on the Lifetimes of Objects* (1983)
- Wilson, *Uniprocessor Garbage Collection Techniques* (1992 survey)

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入分代回收原语：

```
allocate → nursery
minor GC: copy live nursery objects, promote survivors
write barrier: old → young pointer → mark card dirty
remembered set: dirty cards are extra roots for minor GC
```

与 mimalloc（分配）、Mesh（VM remapping）、RCU（宽限期）都不同。

## 学习目标

- 分代假说（大多数对象很快死亡）
- Write barrier + remembered set
- Promotion
- Minor vs major GC

## 第一轮

- `notes/architecture.md`
- 极简 model：nursery + old + card table
- 探针：分配/促进/写屏障记录
