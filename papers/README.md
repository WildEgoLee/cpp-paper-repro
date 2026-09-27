# 论文目录

一篇一个目录。状态只有四种：`queued`、`protocol-frozen`、`running`、`reported`。

| slug | 状态 | 说明 |
| --- | --- | --- |
| `mimalloc-2019` | protocol-frozen | 第一篇。机制探针已能编译。96 组对照还没跑 |
| `thinlto-2017` | queued | 编译 / 链接。协议未写 |
| `bolt-2019` | queued | 链接后的二进制布局。协议未写 |
| `mesh-2019` | queued | 不能移动对象时的压缩。协议未写 |

新论文从 `_template` 复制。不要在 `queued` 的目录里放测量数字。
