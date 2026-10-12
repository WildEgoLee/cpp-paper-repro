// Real io_uring: batch SQEs + registered (fixed) buffers.
// Compares against traditional pread.

#include <linux/io_uring.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>
#include <chrono>

static int io_uring_setup(unsigned entries, struct io_uring_params* p) {
  return static_cast<int>(syscall(__NR_io_uring_setup, entries, p));
}
static int io_uring_enter(int fd, unsigned to_submit, unsigned min_complete, unsigned flags) {
  return static_cast<int>(syscall(__NR_io_uring_enter, fd, to_submit, min_complete, flags, nullptr, 0));
}
static int io_uring_register(int fd, unsigned opcode, void* arg, unsigned nr) {
  return static_cast<int>(syscall(__NR_io_uring_register, fd, opcode, arg, nr));
}

int main() {
  constexpr int N = 32;
  constexpr int BS = 4096;
  const char* path = "/tmp/uring_batch_probe.dat";

  int file_fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (file_fd < 0) { std::perror("open"); return 1; }
  std::vector<char> src(N * BS, 'A');
  if (write(file_fd, src.data(), src.size()) != static_cast<ssize_t>(src.size())) {
    std::perror("seed"); return 1;
  }

  // ---- Traditional pread ----
  std::vector<char> trad(N * BS);
  auto t0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; ++i) {
    if (pread(file_fd, trad.data() + i * BS, BS, i * BS) != BS) {
      std::perror("pread"); return 1;
    }
  }
  auto t1 = std::chrono::high_resolution_clock::now();
  auto trad_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

  // ---- io_uring batch + fixed buffers ----
  struct io_uring_params params{};
  int ring_fd = io_uring_setup(64, &params);
  if (ring_fd < 0) { std::perror("setup"); return 1; }

  std::vector<char> ubufs(N * BS);
  std::vector<struct iovec> iov(N);
  for (int i = 0; i < N; ++i) {
    iov[i].iov_base = ubufs.data() + i * BS;
    iov[i].iov_len = BS;
  }
  if (io_uring_register(ring_fd, IORING_REGISTER_BUFFERS, iov.data(), N) < 0) {
    std::perror("register buffers"); return 1;
  }

  size_t sq_sz = params.sq_off.array + params.sq_entries * sizeof(unsigned);
  size_t cq_sz = params.cq_off.cqes + params.cq_entries * sizeof(struct io_uring_cqe);
  void* sq = mmap(nullptr, sq_sz, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE, ring_fd, IORING_OFF_SQ_RING);
  void* cq = mmap(nullptr, cq_sz, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE, ring_fd, IORING_OFF_CQ_RING);
  void* sqes_ptr = mmap(nullptr, params.sq_entries * sizeof(struct io_uring_sqe),
                        PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE, ring_fd, IORING_OFF_SQES);
  if (sq==MAP_FAILED || cq==MAP_FAILED || sqes_ptr==MAP_FAILED) { std::perror("mmap"); return 1; }

  auto* sq_tail = reinterpret_cast<unsigned*>(static_cast<char*>(sq) + params.sq_off.tail);
  auto* sq_array = reinterpret_cast<unsigned*>(static_cast<char*>(sq) + params.sq_off.array);
  auto* cq_head = reinterpret_cast<unsigned*>(static_cast<char*>(cq) + params.cq_off.head);
  auto* cq_tail = reinterpret_cast<unsigned*>(static_cast<char*>(cq) + params.cq_off.tail);
  auto* cqes = reinterpret_cast<struct io_uring_cqe*>(static_cast<char*>(cq) + params.cq_off.cqes);
  auto* sqes = static_cast<struct io_uring_sqe*>(sqes_ptr);

  unsigned tail = *sq_tail;
  for (int i = 0; i < N; ++i) {
    unsigned idx = (tail + i) & (params.sq_entries - 1);
    struct io_uring_sqe* sqe = &sqes[idx];
    std::memset(sqe, 0, sizeof(*sqe));
    sqe->opcode = IORING_OP_READ_FIXED;
    sqe->fd = file_fd;
    sqe->addr = reinterpret_cast<uint64_t>(iov[i].iov_base);
    sqe->len = BS;
    sqe->off = static_cast<uint64_t>(i) * BS;
    sqe->buf_index = static_cast<uint16_t>(i);
    sqe->user_data = static_cast<uint64_t>(i);
    sq_array[idx] = idx;
  }
  *sq_tail = tail + N;

  auto t2 = std::chrono::high_resolution_clock::now();
  int ret = io_uring_enter(ring_fd, N, N, IORING_ENTER_GETEVENTS);
  auto t3 = std::chrono::high_resolution_clock::now();
  if (ret < 0) { std::perror("enter"); return 1; }
  auto uring_us = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

  int completed = 0;
  unsigned head = *cq_head;
  while (head != *cq_tail && completed < N) {
    if (cqes[head & (params.cq_entries - 1)].res == BS) ++completed;
    ++head;
  }
  *cq_head = head;

  bool data_ok = (std::memcmp(trad.data(), ubufs.data(), src.size()) == 0);

  std::printf("{\n");
  std::printf("  \"probe\": \"io-uring-batch-fixed\",\n");
  std::printf("  \"ops\": %d,\n", N);
  std::printf("  \"block_size\": %d,\n", BS);
  std::printf("  \"traditional_pread_us\": %ld,\n", trad_us);
  std::printf("  \"io_uring_batch_fixed_us\": %ld,\n", uring_us);
  std::printf("  \"completed\": %d,\n", completed);
  std::printf("  \"data_match\": %s,\n", data_ok ? "true" : "false");
  std::printf("  \"note\": \"batch SQE + IORING_REGISTER_BUFFERS + IORING_OP_READ_FIXED vs pread\"\n");
  std::printf("}\n");

  munmap(sq, sq_sz);
  munmap(cq, cq_sz);
  munmap(sqes_ptr, params.sq_entries * sizeof(struct io_uring_sqe));
  close(ring_fd);
  close(file_fd);
  unlink(path);

  return (completed == N && data_ok) ? 0 : 1;
}
