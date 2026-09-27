# ThinLTO，第二篇

协议还没写。已经能从论文正文锁住的事实在 [lock/environment-paper.json](lock/environment-paper.json)。这里只保留还没做完的顺序。

P0 还剩四步，按这个顺序，不跳：

1. 继续找精确 revision 和脏文件脚本。论文自己没给 commit。找到了也只能标成外部 artifact 重建的 pin。
2. 再锁 Clang 和 Chromium 的源码 revision。Ad Delivery 是私有负载，不替换。
3. 再锁 gold、binutils、CMake、Ninja 和构建参数。
4. 最后才写 claim 和矩阵。

增量实验的具体 path / function 仍然是 unresolved。2016-06-21 的 LLVM blog 写了 `DenseMap::grow()` 和 `InstCombineCalls.cpp` 里的 `visitCallInst()`。那是外部候选，不是论文原文，不能写成 Exact workload。
