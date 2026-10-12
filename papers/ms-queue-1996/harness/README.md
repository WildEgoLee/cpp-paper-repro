# MS-queue-1996 harness

Michael-Scott queue with:

- Tagged pointers (ABA on the pointer word)
- **Hazard pointer** safe reclamation

A thread publishes the node it is about to touch into a hazard pointer slot.
`retire` only deletes a node after a scan confirms no hazard pointer still
references it.

```bash
make -C papers/ms-queue-1996/harness
./papers/ms-queue-1996/harness/mechanism_probe
```

Toy domain (fixed slots, simple scan). Sufficient to illustrate the
reclamation protocol used by production lock-free structures.
