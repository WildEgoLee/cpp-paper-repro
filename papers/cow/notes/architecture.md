# Copy-on-Write Architecture Notes

学习目标：延迟复制。

## 核心对象

```
Page
  data
  refcount
Holder
  pointer to Page
  writable?        // false while shared
```

## Ownership

- 多个 holder 可以共享同一个 Page。
- 写入时如果 refcount > 1，写者获得新的私有副本。
- refcount == 1 时可以直接写，无需复制。

## Fast path / Slow path

- Fast：读（共享无开销）；或单一所有者写。
- Slow：共享时写 → 分配 + 复制。

## Invariants

1. 两个 holder 指向同一 Page 时，该 Page 对两者都是只读。
2. 写入不会影响其他 holder。
3. refcount == 0 时 Page 可回收。

## Policy / Mechanism 分离

- Policy：何时共享、何时回收。
- Mechanism：引用计数、写时复制、页表权限。

## Concurrency boundary

写时的复制必须与其他读者同步；或者用页错误停顿写者。

## State 位置

- 数据：物理页。
- 共享关系：refcount + 页表权限。

## Data layout 理由

页粒度复制让大多数读无开销，只有写的页才付出复制代价。

## Failure modes

- 忘记降 refcount → 泄漏。
- 写时没检查 refcount → 破坏其他 holder。
- 过度共享 + 频繁写 → 比直接复制更慢。

## 与其他模型对比

| | 共享 | 写开销 |
|--|------|--------|
| Mutable | 无 | 立即 |
| Persistent | 结构共享 | 路径复制 |
| CoW | 页共享 | 写时复制 |
