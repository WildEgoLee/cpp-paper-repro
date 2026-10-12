# Actor Model

Carl Hewitt, Peter Bishop, Richard Steiger. *A Universal Modular ACTOR Formalism for Artificial Intelligence*. IJCAI 1973.

Later developed by Gul Agha.

- Original: https://www.ijcai.org/Proceedings/73/Papers/027B.pdf
- Agha, *Actors: A Model of Concurrent Computation in Distributed Systems* (MIT Press, 1986)

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入消息传递并发原语：

```
Actor
  ├─ private state
  ├─ mailbox (queue of messages)
  └─ behavior: on message → send / create / become
```

无共享内存竞争。与 RCU、MS queue、work-stealing 的共享内存模型形成对比。

## 学习目标

- 私有状态 + 异步消息
- 三个原语：create / send / become
- 邮箱与调度
- 与共享内存并发的权衡

## 第一轮

- `notes/architecture.md`
- 极简 model：Actor + mailbox
- 探针：多 actor 互相发消息并正确处理
