# Mesh: Compacting Memory Management for C/C++ Applications

状态：`queued`。协议未冻结。这里没有测量数字。

Bobby Powers, David Tench, Emery D. Berger, Andrew McGregor. *Mesh: Compacting Memory Management for C/C++ Applications*. PLDI 2019.

C/C++ 对象不能搬家。Mesh 用虚拟内存把物理页合并掉，虚拟地址保持不变。官方实现公开。这篇比 mimalloc 更依赖虚拟内存行为，放在 allocator 测量闭环建立之后。

等前一篇的测量闭环稳定后再开。
