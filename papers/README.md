# 论文目录

一篇一个目录。状态只有四种：`queued`、`protocol-frozen`、`running`、`reported`。

| slug | 状态 | 说明 |
| --- | --- | --- |
| `mimalloc-2019` | protocol-frozen | 第一篇。runner 已就绪。等待至少 12 个硬件线程的 Linux 做 smoke |
| `thinlto-2017` | protocol-frozen | 第二篇。P1 已关闭。Exact 不可测。P1.5 未开始，不许跑 |
| `bolt-2019` | queued | 链接后的二进制布局。协议未写 |
| `mesh-2019` | queued | 不能移动对象时的压缩。协议未写 |

新论文从 `_template` 复制。不要在 `queued` 的目录里放测量数字。
