# 复现方法

每篇论文走同一条流水线。没有写进该论文 `PROTOCOL.md` 的步骤，不算做过。

```text
论文
  → 抽出 claim，并写成可失败的判据
  → 记下论文当时的实现版本和机器，和今天的版本分开
  → 基线 allocator / 编译器
  → benchmark
  → 关掉单个机制的 ablation
  → 性能计数器，而不只是墙钟
  → Exact / Directional / Mechanistic 三档结论
```

## 三档

Exact。论文表格里的百分比或倍数，在误差范围内重现。机器、编译器、库版本都对不上时，这一档通常不成立，而且 **不成立是预期结果**，不是实验失败。

Directional。谁更快、线程数上去之后谁扩展得更好、RSS 有没有跟着爆炸。排序和趋势与论文一致即可，不要求同一个百分比。

Mechanistic。把论文点名的机制关掉或单独做出来之后，对应 workload 朝预测的方向变。只看到「mimalloc 赢了」不够。要能区分「因为论文描述的机制赢了」。

三档都要写。禁止用其中一档的结果去填另一档。

## 什么时候算协议冻结

一篇论文的目录里同时有这些东西，才叫协议冻结，而不是「还在读」：

- 引用和要对照的实现版本
- 编号的 claim，每条都有失败条件
- 第一轮要跑的 workload，以及明确不跑的东西
- 重复次数、统计量、计数器
- ablation 预期方向
- 论文环境与本机环境的差异列表

冻结之后再跑。跑完再解释，顺序不能反。

## 目录约定

```text
papers/<slug>/
  README.md        状态、引用、这一篇现在做到哪
  PROTOCOL.md      冻结后的实验协议
  harness/         可以在没有上游仓库的情况下编译的机制探针
  bench/           上游 benchmark 的矩阵和版本钉死
  results/         带机器说明的结果；原始多次运行放 results/runs/，不入库
```

`results/runs/` 被 gitignore。入库的 JSON 必须带 disclaimer，写明它是机制探针、论文复现，还是现代版本的 replication。
