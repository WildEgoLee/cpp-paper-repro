# 协议：thinlto-2017

状态：`protocol-frozen`。冻结日期以本文件进入仓库的提交为准。这里没有 checkout，没有 runner，也没有测量数字。执行还不允许。

P0 的论文事实在 [lock/environment-paper.json](lock/environment-paper.json)，不再重开。P1.5 以后可以另选公开 checkout 和工具链，但必须写成 `pin_class: operationalization`、`supports_exact: false`。那个选择不能改下面的失败判据。

## 不在本协议里的实验

§8.1 的 SPEC cpu2006 运行时，以及 §8.4 的 Ad Delivery 分布式构建，都不进入这三条 claim。Ad Delivery 是私有负载，不许用别的程序顶替。

## 两个实验不要合成一根线程轴

| 实验 | 论文位置 | 并行度 | 对照 |
| --- | --- | --- | --- |
| E-scale | §8.2 | 8、16、32，各点分开 | ThinLTO、LLVM LTO、GCC LTO；Clang 和 Chromium |
| E-incr | §8.3 | 只有 `-j32` | LLVM LTO、ThinLTO、No-LTO；三种编辑状态 |

E-scale 不使用 `-j32` 这一个点代表整条曲线。E-incr 也不再扫 8 和 16。

## Exact 配置里未解决的冲突

§8.3 正文写 No-LTO 为 `-O2`。Figure 7 图注写 `-O3`，图内是 `-O3 -g0`。这是 `exact-configuration: unresolved`。协议不选边。以后的 operationalization 如果必须取一边，要在自己的锁里写明取了哪一边，并且不能因此把结果叫成 Exact。

编译器精确 revision、Clang / Chromium 的论文 checkout、两个脏编辑的路径和函数，都已经是 `unresolved-after-search`。依赖它们才能回答的 Exact 项写成 `not-testable-from-public-artifacts`，不是 pending。gold、binutils、CMake、Ninja 的小版本是 `paper-unspecified`，只影响 Exact fidelity。

## Claim

三条都要能失败。没有数值门槛。论文图里的分钟数和内存数不是判据。

### C1 构建扩展性

在 Clang 和 Chromium 这种规模的程序上，ThinLTO 的 build/link 时间随并行度的变化和 Full LTO 不同。LLVM LTO 的优化阶段是单线程的，只有代码生成并行；ThinLTO 的后端本身并行。GCC LTO 只出现在 E-scale，不出现在 E-incr。

Exact：`not-testable-from-public-artifacts`。公开材料没有能对上 §8.2 的 checkout，图上的时间也不能当门槛。

Directional：E-scale 的 8、16、32 三点上，ThinLTO 的耗时应随着线程增加而下降，并且比同一点的 LLVM LTO 和 GCC LTO 更跟得上线程数。失败：三点上 ThinLTO 的耗时不随线程下降，或者它和 Full LTO 贴在同一条不扩展的曲线上。只在一个线程数上更快，不够。

### C2 内存扩展性

比较的是链接期间的内存压力，不是运行时 RSS。指标是进程的 `resident set size - shared memory` 的峰值。论文用它，是因为只读 mmap 会留在 RSS 里，但不算系统上的内存压力。

`ru_maxrss` 和 `/usr/bin/time` 的 `%M` 都不是这个指标。用了它们而仍然声称测到论文的内存，算测量无效，不产生 C2 的结论。

Exact：`not-testable-from-public-artifacts`。

Directional：在 E-scale 上，ThinLTO 的该项峰值稳定地低于同一程序、同一线程数的 LLVM LTO；在论文也给出 GCC 的序列步上，也低于 GCC LTO。失败：ThinLTO 的 `RSS - shared` 峰值没有稳定地更低。只看到时间变好，不算这条成立。

### C3 增量性

E-incr 有三种状态，不能合并：

1. clean：从空的构建目录完整构建。
2. frequent-header-change：改一个被广泛引用的 header 函数，迫使大量源文件重编，然后做链接。
3. infrequent-source-change：改一个很少被调用的实现文件函数，增量时间应当短。

ThinLTO 自己的 backend cache 是机制的一部分。cache key 是模块 IR 的 hash，加上 thin link 的分析结果；命中时直接复用 native object。测增量时这个 cache 必须开着，并且是 warm。ccache、sccache 以及其他额外编译缓存必须关。关掉 ThinLTO 自己的 cache，测到的就不是论文里的增量 ThinLTO，结果作废。

`DenseMap::grow()` 和 `InstCombineCalls.cpp` 里的 `visitCallInst()` 来自更早的另一台机器，不是 §8.3 的 Exact 编辑。它们只能出现在以后的 Directional operationalization 里。

Exact：`not-testable-from-public-artifacts`。论文没有给出文件名和函数名，优化级别冲突也没有裁决。

Directional：在后两种局部修改上，ThinLTO 的增量耗时接近 No-LTO，而 LLVM LTO 仍然接近一次完整的 LTO 链接。失败：局部修改之后，ThinLTO 的增量耗时和 LLVM LTO 一样，没有留下可复用的后端工作。clean 上 ThinLTO 比 No-LTO 稍慢或稍快，都不单独否决这条；否决看的是局部修改之后还保不保得住增量。

## 证据等级

| 等级 | 现在能做什么 |
| --- | --- |
| Exact | 三条都是 `not-testable-from-public-artifacts`。不因为后来选了一个公开 checkout 就改回可测 |
| Directional | 判据已经冻结。要等 P1.5 的 operationalization 锁，而且那把锁不能改判据 |
| Mechanistic | 可以在任何以后的运行里检查，不依赖 2016 年的 checkout：内存用的是不是 `RSS - shared`；增量跑有没有留着 ThinLTO backend cache、有没有混进 ccache/sccache；E-scale 和 E-incr 有没有被收成同一根线程轴 |

机制检查失败时，对应 claim 没有结果，不是“方向相反”。

## 明确不做什么

- 不克隆 LLVM 或 Chromium，不写 runner，不产生 benchmark 数字。
- 不把 `llvmorg-4.0.0`、`llvm-3.9.0` 或 `gcc-7.1.0` 补成论文编译器。
- 不把 2016-09-12 的 `use_thin_lto`，或任何一个仅因日期接近而被选中的 commit，写成论文 checkout。
- 不把 ES-2690 改写成 E5-2690。论文机器字符串保持原样；它不是本协议的执行主机要求。
