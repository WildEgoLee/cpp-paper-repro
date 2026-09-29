# ThinLTO，第二篇

P1 已关闭。claim 在 [PROTOCOL.md](PROTOCOL.md)，不再改。

P1.5 还没开始。它如果以后写，只回答怎么执行已经冻结的 Directional 协议，不回答论文当时用了哪一次 checkout。那把锁里可以有：公开 source checkout 和选择理由，`pin_class: operationalization`、`supports_exact: false`；实际的 clang、GCC、gold、CMake、Ninja 版本；E-scale 和 E-incr 两套分开的 build recipe；`-O2` / `-O3` 冲突实际采用哪一支，并标明这是 operationalization；C2 的 `/proc` 采样算法和频率；C3 的 cache 目录生命周期、warm 状态怎么建立、dirty edit 是哪一处。

也可以规定重复次数、取 median、噪声处理和 artifact provenance。不能新增协议里没有的成败条件。例如可以写每个点跑 5 次取 median，不能写“下降至少 10% 才算下降”。Exact 保持 `not-testable-from-public-artifacts`。

在那之前：不克隆，不写 runner，不跑。
