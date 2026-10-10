# cpp-paper-repro

C++ / native 系统论文的可复现性工作台。一篇论文一个目录。

不把「我这台机器不是论文里的 +14%」写成不可复现。先冻结 claim，再分成三档记录：

| 档位 | 问题 |
| --- | --- |
| Exact | 数值是否接近论文 |
| Directional | 排序和趋势是否一致 |
| Mechanistic | 性能变化是否符合论文给出的机制 |

2026 年的 CPU、内核、glibc、编译器和 allocator 都已经不是论文当年那一套。机制还在不在，和历史数字是否重合，是两件分开的事。方法说明见 [docs/method.md](docs/method.md)。

## 系列

| 目录 | 论文 | 状态 |
| --- | --- | --- |
| [papers/mimalloc-2019](papers/mimalloc-2019) | Mimalloc: Free List Sharding in Action, APLAS 2019 | 第一篇，协议已冻结，机制探针可编译 |
| [papers/thinlto-2017](papers/thinlto-2017) | ThinLTO: Scalable and Incremental LTO, CGO 2017 | protocol-frozen |
| [papers/bolt-2019](papers/bolt-2019) | BOLT: A Practical Binary Optimizer for Data Centers and Beyond, CGO 2019 | protocol-frozen（下一篇，机制探针待加） |
| [papers/mesh-2019](papers/mesh-2019) | Mesh: Compacting Memory Management for C/C++ Applications, PLDI 2019 | protocol-frozen，增强机制探针可运行 |

范围只包括 C / C++ 工程体系（运行时、内存、编译、链接、二进制布局）。WhisperX 不属于这个仓库。

## 第一篇，现在就能跑的部分

`papers/mimalloc-2019/harness` 里的探针 **不链接** mimalloc、jemalloc 或 tcmalloc。

```bash
make -C papers/mimalloc-2019/harness
./papers/mimalloc-2019/harness/mechanism_probe
```

Mesh 增强探针：

```bash
make -C papers/mesh-2019/harness
./papers/mesh-2019/harness/mechanism_probe
```

## 加一篇论文

复制 [papers/_template](papers/_template)。不要把结果写进别的论文目录，也不要在协议冻结之前填「已复现」。
