# Skip List

William Pugh. *Skip Lists: A Probabilistic Alternative to Balanced Trees*. CACM 1990.

- Paper: https://epaperpress.com/sortsearch/download/skiplist.pdf

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入概率平衡原语：

```
Level 0: all nodes (sorted linked list)
Level i: ~p of the nodes from level i-1 also appear
Forward[i]: points to next node of level >= i

Insert: choose random height, splice locally
Search: drop down when next key is too big
```

不需要旋转或页分裂就能保持预期 O(log N)。与 B+ Tree 的确定性结构完全不同。

## 学习目标

- 随机高度
- 多层前向指针
- 局部修改（插入/删除只影响邻居）
- 与确定性平衡树的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：随机高度 + 多层链表
- 探针：插入后查找，高层指针跳过部分节点
