// G-Counter merge probe.
// Demonstrates that pointwise-max merge is commutative, associative, and idempotent.

#include "../model/minimal/gcounter.hpp"
#include <cstdio>

int main() {
  // Two replicas, independent increments
  GCounter a(2, 0), b(2, 1);
  a.increment(3);
  b.increment(5);

  a.merge(b);
  b.merge(a);

  bool commutative = a.same_state(b) && a.value() == 8;

  // Idempotence
  GCounter c = a;
  c.merge(a);
  bool idempotent = c.same_state(a);

  // Associativity: (x ⋄ y) ⋄ z == x ⋄ (y ⋄ z)
  GCounter x(2, 0), y(2, 1), z(2, 0);
  x.increment(1);
  y.increment(2);
  z.increment(4);

  GCounter left = x; left.merge(y); left.merge(z);
  GCounter right = y; right.merge(z); right.merge(x);
  bool associative = left.same_state(right);

  bool ok = commutative && idempotent && associative;

  std::printf("{\n");
  std::printf("  \"probe\": \"crdt-gcounter\",\n");
  std::printf("  \"commutative\": %s,\n", commutative ? "true" : "false");
  std::printf("  \"idempotent\": %s,\n", idempotent ? "true" : "false");
  std::printf("  \"associative\": %s,\n", associative ? "true" : "false");
  std::printf("  \"merged_value\": %d,\n", a.value());
  std::printf("  \"note\": \"pointwise max is a join-semilattice; merge needs no coordination\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
