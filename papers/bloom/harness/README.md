# Bloom Filter harness

## Standard Bloom

`mechanism_probe` inserts a few keys and shows no false negatives plus some false positives.

```bash
make -C papers/bloom/harness mechanism_probe
./papers/bloom/harness/mechanism_probe
```

## False-positive growth

`fpr_probe` inserts more keys and measures how many absent keys are reported present. The rate rises as bits fill.

```bash
make -C papers/bloom/harness fpr_probe
./papers/bloom/harness/fpr_probe
```

## Counting Bloom

`counting_probe` uses counters so `erase` is possible. After erase the key is reported absent; re-insert brings it back.

```bash
make -C papers/bloom/harness counting_probe
./papers/bloom/harness/counting_probe
```

All toys. They demonstrate the membership trade-off and the deletion extension.
