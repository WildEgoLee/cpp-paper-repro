# ThinLTO: Scalable and Incremental LTO

状态：`queued`。协议未冻结。这里没有测量数字。

Teresa Johnson, Mehdi Amini, Xinliang David Li. *ThinLTO: Scalable and Incremental LTO*. CGO 2017.

LLVM 今天仍然直接支持 ThinLTO。这篇要看的是：跨模块优化还在的时候，构建扩展性是否更接近普通编译，而不是再做一遍完整 LTO。和 mimalloc 互补：那篇是运行时 / 内存，这篇是构建 / 链接 / 优化。

等 mimalloc 的第一轮对照跑完再写协议。不要提前把 LLVM 构建系统搬进这个目录。
