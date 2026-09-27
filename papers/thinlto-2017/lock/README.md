# lock

[environment-paper.json](environment-paper.json) 只锁论文里已经写明、而且不该被“修正”的事实。没有 claim，没有矩阵，没有测量数字。LLVM 和 Chromium 都没有克隆。

还没锁住的东西：精确编译器 revision、Clang / Chromium 的源码 revision、gold / binutils / CMake / Ninja 的版本，以及增量实验改的具体文件和函数。这些不齐，就不写 `PROTOCOL.md`。
