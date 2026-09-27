"""Execution semantics for the mimalloc-2019 round-1 matrix.

This package resolves the frozen matrix into an immutable plan. It does not
run allocators and it does not emit benchmark numbers.
"""

SCHEMA_PLAN = "cpp-paper-repro.plan/v1"
SCHEMA_HOST = "cpp-paper-repro.host/v1"
SCHEMA_ARTIFACTS = "cpp-paper-repro.artifacts/v1"
SCHEMA_CASE = "cpp-paper-repro.case/v1"

# Fixed seed for the within-block shuffle. Changing it changes order only.
PLAN_SEED = 20190621
