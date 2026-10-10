# BOLT Architecture Notes

学习目标：一个 binary optimizer 内部如何建模问题。

## 核心对象

```
BasicBlock { id, execution_count, successors }
Edge { src, dst, count }
CFG
Profile { edge → count }
Layout { block order }
```

## Ownership

- CFG 拥有所有 BasicBlock。
- Profile 是只读输入。
- LayoutStrategy 产出新的 Layout，不修改原 CFG。

## Fast path / Slow path

- Fast: fall-through 热边（无条转）。
- Slow: 跨冷块的 taken branch。

## Invariants

1. Layout 不改变语义（控制流等价）。
2. 热边应尽量 fall-through。
3. 冷块不应打断热路径的 I-cache 局部性。

## Policy / Mechanism 分离

- Policy: HotPathLayout / RandomLayout / OriginalLayout。
- Mechanism: 重排基本块、修补跳转、写回 ELF。

## Concurrency boundary

BOLT 本身是离线工具；并发性在 profile 采集阶段。

## State 位置

- 执行计数在 Profile。
- 最终顺序在 Layout。
- 二进制布局在 ELF section。

## Data layout 理由

把热路径的基本块放在一起，减少 I-cache miss 和分支错误预测。

## Failure modes

- Profile 不准 → 错误重排反而变慢。
- 间接跳转无法重建 CFG → 保守不重排。

## 与上游差异

探针只演示了布局效应（热路径 vs 冷块交错）。
缺少真实 CFG 建模、多种 LayoutStrategy、ELF 重写。
