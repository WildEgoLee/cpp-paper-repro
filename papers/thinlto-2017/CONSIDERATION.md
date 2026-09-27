# ThinLTO，第二篇

`P0-paper-facts` 已关闭。源码检索也停了：Clang 和 Chromium 的 `paper_source_revision` 都是 `unresolved-after-search`。没有 reconstructed pin，也没有挑一个同期 commit 当成 operationalization。协议仍然不写。

2016 年 11 月作者幻灯片里的 IR 规模和论文对得上：Clang 1,938 个 IR 文件、`-g0` 下 217 MB；Chromium 17,798 个文件、706 MB。构建图用的机器写成 E5-2690。幻灯片没有 SVN、DEPS 或 CMakeCache。增量那一页是另一台 2013 Mac Pro，不能拿来补 §8.3 的路径。

Chromium 2016 年 8 月的公开参数是 full LTO。9 月的 `use_thin_lto` 和 ToT bot 是事后评估。12 月把 jobs 限到 8，也不是论文的 8/16/32。这些都不是 pin。

gold、binutils、CMake、Ninja 的版本停在 `paper-unspecified`，不再搜。

Exact 矩阵现在不可测。以后如果要做 Directional，必须另写一个 `pin_class: operationalization` 的 checkout，并标 `supports_exact: false`。那一步还没发生。
