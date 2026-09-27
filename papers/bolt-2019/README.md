# BOLT: A Practical Binary Optimizer for Data Centers and Beyond

状态：`queued`。协议未冻结。这里没有测量数字。

Maksim Panchenko, Rafael Auler, Bill Nell, Guilherme Ottoni. *BOLT: A Practical Binary Optimizer for Data Centers and Beyond*. CGO 2019.

做的是链接之后的 ELF：采样、再改二进制布局。依赖 Linux perf 和 profile，环境要求比 mimalloc 的机制探针高，不作为第一篇。

等前一篇的测量闭环稳定后再开。
