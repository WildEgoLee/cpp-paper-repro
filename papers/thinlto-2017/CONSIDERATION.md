# ThinLTO，第二篇

`P0-paper-facts` 已关闭。`P0-artifact-reconstruction` 还在进行。协议仍然不写。

搜索只接受四类来源：论文、作者同期材料、明确的 benchmark / build artifact、可核对的历史构建记录。这些都没有时，记成 `unresolved-after-search`。不因为日期接近就挑一个 commit。

Clang 和 Chromium 的源码 revision，以及真正定义实验的构建参数，在写成矩阵之前还要继续找。2016-09-12 的 Chromium `use_thin_lto` 只是开始评估的开关，默认关闭，不是 §8.2 的 checkout。

gold、binutils、CMake、Ninja 的小版本如果最终没有出处，可以冻成 `paper-unspecified`，以后的选择单独记成 `project_operationalization`。它们影响 Exact，但不该把 Directional 协议永久卡住。

编译器精确 revision 和两个脏函数也一样。找不到时，需要它们的 Exact claim 标成不可测。Directional / Mechanistic 仍可以写协议。那一步还没到。2016-06-21 的 LLVM blog 用的是 20 核 E5-2680 v2，不是论文的 16 核 ES-2690，所以那两个函数名仍然只是更早一次实验的候选。
