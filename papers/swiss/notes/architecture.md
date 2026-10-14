# Swiss Table Architecture Notes

学习目标：控制字节与组探测。

## 核心对象

```
control[n]     // 1 byte per slot: empty / deleted / fingerprint
slots[n]       // real keys
```

## Ownership

- 控制字节与真键分离存储。
- 探测只触控制字节；只有指纹匹配才加载真键。

## Fast path / Slow path

- Fast：组内 SIMD/SWAR 比较指纹，过滤绝大多数槽。
- Slow：指纹匹配后加载真键比较。

## Invariants

1. 控制字节为 empty 时，探测可停。
2. 指纹不匹配的槽不会被加载真键。
3. 删除用 tombstone，不中断探测。

## Policy / Mechanism 分离

- Policy：组大小（8/16）、负载因子。
- Mechanism：控制字节 + 组探测。

## Concurrency boundary

单线程玩具；真实实现可以做细粒度锁。

## State 位置

- 控制字节：紧凑数组，缓存友好。
- 真键：另一数组，只在需要时加载。

## Data layout 理由

控制字节让探测只触缓存行，真键加载被推迟到确认匹配。

## Failure modes

- 指纹碰撞 → 假阳性候选，仍需真键比较。
- 负载过高 → 探测组数增加。

## 与 Robin Hood 对比

| | 探测 | 元数据 | 删除 |
|--|------|--------|------|
| Robin Hood | 单槽 + PSL | 键与 PSL 同槽 | 移动 |
| Swiss | 组 + 指纹 | 控制字节分离 | tombstone |
