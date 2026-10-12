// Real io_uring probe (raw syscalls + mmap).
// Submits IORING_OP_NOP and receives the CQE.
// Linux only. Requires kernel with io_uring.

#include <linux/io_uring.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cerrno>

static int io_uring_setup(unsigned entries, struct io_uring_params* p) {
  return static_cast<int>(syscall(__NR_io_uring_setup, entries, p));
}

static int io_uring_enter(int fd, unsigned to_submit, unsigned min_complete, unsigned flags) {
  return static_cast<int>(syscall(__NR_io_uring_enter, fd, to_submit, min_complete, flags, nullptr, 0));
}

int main() {
  struct io_uring_params params{};
  int ring_fd = io_uring_setup(8, &params);
  if (ring_fd < 0) {
    std::perror("io_uring_setup");
    return 1;
  }

  size_t sq_ring_sz = params.sq_off.array + params.sq_entries * sizeof(unsigned);
  size_t cq_ring_sz = params.cq_off.cqes + params.cq_entries * sizeof(struct io_uring_cqe);

  void* sq_ptr = mmap(nullptr, sq_ring_sz, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_POPULATE, ring_fd, IORING_OFF_SQ_RING);
  void* cq_ptr = mmap(nullptr, cq_ring_sz, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_POPULATE, ring_fd, IORING_OFF_CQ_RING);
  void* sqes_ptr = mmap(nullptr, params.sq_entries * sizeof(struct io_uring_sqe),
                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
                        ring_fd, IORING_OFF_SQES);

  if (sq_ptr == MAP_FAILED || cq_ptr == MAP_FAILED || sqes_ptr == MAP_FAILED) {
    std::perror("mmap");
    return 1;
  }

  auto* sq_tail = reinterpret_cast<unsigned*>(static_cast<char*>(sq_ptr) + params.sq_off.tail);
  auto* sq_array = reinterpret_cast<unsigned*>(static_cast<char*>(sq_ptr) + params.sq_off.array);
  auto* cq_head = reinterpret_cast<unsigned*>(static_cast<char*>(cq_ptr) + params.cq_off.head);
  auto* cq_tail = reinterpret_cast<unsigned*>(static_cast<char*>(cq_ptr) + params.cq_off.tail);
  auto* cqes = reinterpret_cast<struct io_uring_cqe*>(static_cast<char*>(cq_ptr) + params.cq_off.cqes);
  auto* sqes = static_cast<struct io_uring_sqe*>(sqes_ptr);

  // Submit NOP
  unsigned tail = *sq_tail;
  unsigned index = tail & (params.sq_entries - 1);
  struct io_uring_sqe* sqe = &sqes[index];
  std::memset(sqe, 0, sizeof(*sqe));
  sqe->opcode = IORING_OP_NOP;
  sqe->user_data = 0x1234;
  sq_array[index] = index;
  *sq_tail = tail + 1;

  int ret = io_uring_enter(ring_fd, 1, 1, IORING_ENTER_GETEVENTS);
  if (ret < 0) {
    std::perror("io_uring_enter");
    return 1;
  }

  unsigned head = *cq_head;
  bool got = false;
  uint64_t ud = 0;
  int res = -1;
  if (head != *cq_tail) {
    struct io_uring_cqe* cqe = &cqes[head & (params.cq_entries - 1)];
    ud = cqe->user_data;
    res = cqe->res;
    *cq_head = head + 1;
    got = true;
  }

  std::printf("{\n");
  std::printf("  \"probe\": \"io-uring-real\",\n");
  std::printf("  \"sq_entries\": %u,\n", params.sq_entries);
  std::printf("  \"cq_entries\": %u,\n", params.cq_entries);
  std::printf("  \"nop_completed\": %s,\n", got ? "true" : "false");
  std::printf("  \"user_data\": %llu,\n", static_cast<unsigned long long>(ud));
  std::printf("  \"res\": %d,\n", res);
  std::printf("  \"note\": \"real io_uring via syscall + mmap; NOP only\"\n");
  std::printf("}\n");

  munmap(sq_ptr, sq_ring_sz);
  munmap(cq_ptr, cq_ring_sz);
  munmap(sqes_ptr, params.sq_entries * sizeof(struct io_uring_sqe));
  close(ring_fd);

  return (got && ud == 0x1234 && res == 0) ? 0 : 1;
}
