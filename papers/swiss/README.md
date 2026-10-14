# Swiss Table

Google Abseil / Rust hashbrown. Design notes: https://abseil.io/about/design/swisstables

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入控制字节 + 组探测原语：

```
control[i]:  1xxxxxxx = empty/deleted
             0hhhhhhh = full, h = 7-bit hash fingerprint

lookup(x):
  start at group of H1
  SIMD/SWAR compare H2 against 8/16 control bytes at once
  only load real keys for matching fingerprints
```

控制字节让探测只触缓存友好的字节数组，真键比较被推迟。与 Robin Hood 的 PSL 偷窃完全不同。

## 学习目标

- 控制字节与真键分离
- 组探测（SIMD/SWAR）
- 为什么探测不触真键
- 与普通开放寻址的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：控制字节 + 组扫描
- 探针：插入后查找，控制字节过滤
