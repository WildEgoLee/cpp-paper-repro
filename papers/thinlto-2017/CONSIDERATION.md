# ThinLTO，第二篇

考古已经停了。claim 和失败判据在 [PROTOCOL.md](PROTOCOL.md)，并且已经冻结。

P1.5 还没开始。真要跑 Directional 实验时，另写一把 operationalization 锁：公开 checkout、工具链和 build recipe，标明 `pin_class: operationalization`、`supports_exact: false`。那把锁不能改协议里的判据，也不能把 Exact 从 `not-testable-from-public-artifacts` 改成可测。

在那之前：不克隆，不写 runner，不跑。
