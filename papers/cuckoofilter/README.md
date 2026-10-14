# Cuckoo Filter

Bin Fan, Dave Andersen, Michael Kaminsky, Michael Mitzenmacher. *Cuckoo Filter: Practically Better Than Bloom*. CoNEXT 2014.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入指纹与踢出原语：

```
buckets[n] store fingerprints
i2 = i1 XOR hash(fingerprint)

insert: try both buckets; if full, kick out and reinsert
delete: remove fingerprint from either bucket
```

支持删除。与 Bloom Filter 的位图完全不同。

## 学习目标

- 指纹
- 为什么用 XOR 派生第二个桶
- 踢出链
- 与 Bloom Filter 的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：指纹 + 踢出
- 探针：插入后查找，删除后不在
