# Persistent Data Structures

Path copying and structural sharing.

Classic references:
- Chris Okasaki, *Purely Functional Data Structures* (1998)
- Driscoll, Sarnak, Sleator, Tarjan, *Making Data Structures Persistent* (JCSS 1989)

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入不可变版本原语：

```
update(old_root, key, value)
  → copy path from root to leaf
  → new_root shares unchanged subtrees
  → old_root still valid
```

与前面所有可变并发模型都不同。

## 学习目标

- Path copying
- Structural sharing
- 多版本共存
- 不可变性带来的并发优势

## 第一轮

- `notes/architecture.md`
- 极简 model：持久化二叉搜索树（path copy）
- 探针：更新后旧版本仍可查
