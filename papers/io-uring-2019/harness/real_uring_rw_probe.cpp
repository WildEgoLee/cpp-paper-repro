// Real io_uring file read/write probe.
// Uses raw syscalls + mmap. Writes a file then reads it back via io_uring.

#include <linux/io_uring.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>

static int io_uring_setup(unsigned entries, struct io_uring_params* p) {
  return static_cast<int>(syscall(__NR_io_uring_setup, entries, p));
}

static int io_uring_enter(int fd, unsigned to_submit, unsigned min_complete, unsigned flags) {
  return static_cast<int>(syscall(__NR_io_uring_enter, fd, to_submit, min_complete, flags, nullptr, 0));
}

struct Uring {
  int ring_fd = -1;
  struct io_uring_params params{};
  void* sq_ptr = nullptr;
  void* cq_ptr = nullptr;
  void* sqes_ptr = nullptr;
  size_t sq_ring_sz = 0;
  size_t cq_ring_sz = 0;

  unsigned* sq_tail = nullptr;
  unsigned* sq_array = nullptr;
  unsigned* cq_head = nullptr;
  unsigned* cq_tail = nullptr;
  struct io_uring_cqe* cqes = nullptr;
  struct io_uring_sqe* sqes = nullptr;

  bool setup(unsigned entries) {
    ring_fd = io_uring_setup(entries, &params);
    if (ring_fd < 0) return false;

    sq_ring_sz = params.sq_off.array + params.sq_entries * sizeof(unsigned);
    cq_ring_sz = params.cq_off.cqes + params.cq_entries * sizeof(struct io_uring_cqe);

    sq_ptr = mmap(nullptr, sq_ring_sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, ring_fd, IORING_OFF_SQ_RING);
    cq_ptr = mmap(nullptr, cq_ring_sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, ring_fd, IORING_OFF_CQ_RING);
    sqes_ptr = mmap(nullptr, params.sq_entries * sizeof(struct io_uring_sqe),
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, ring_fd, IORING_OFF_SQES);
    if (sq_ptr == MAP_FAILED || cq_ptr == MAP_FAILED || sqes_ptr == MAP_FAILED) return false;

    sq_tail = reinterpret_cast<unsigned*>(static_cast<char*>(sq_ptr) + params.sq_off.tail);
    sq_array = reinterpret_cast<unsigned*>(static_cast<char*>(sq_ptr) + params.sq_off.array);
    cq_head = reinterpret_cast<unsigned*>(static_cast<char*>(cq_ptr) + params.cq_off.head);
    cq_tail = reinterpret_cast<unsigned*>(static_cast<char*>(cq_ptr) + params.cq_off.tail);
    cqes = reinterpret_cast<struct io_uring_cqe*>(static_cast<char*>(cq_ptr) + params.cq_off.cqes);
    sqes = static_cast<struct io_uring_sqe*>(sqes_ptr);
    return true;
  }

  bool submit_and_wait(const struct io_uring_sqe& req, struct io_uring_cqe* out) {
    unsigned tail = *sq_tail;
    unsigned index = tail & (params.sq_entries - 1);
    sqes[index] = req;
    sq_array[index] = index;
    *sq_tail = tail + 1;

    int ret = io_uring_enter(ring_fd, 1, 1, IORING_ENTER_GETEVENTS);
    if (ret < 0) return false;

    unsigned head = *cq_head;
    if (head == *cq_tail) return false;
    *out = cqes[head & (params.cq_entries - 1)];
    *cq_head = head + 1;
    return true;
  }

  ~Uring() {
    if (sq_ptr && sq_ptr != MAP_FAILED) munmap(sq_ptr, sq_ring_sz);
    if (cq_ptr && cq_ptr != MAP_FAILED) munmap(cq_ptr, cq_ring_sz);
    if (sqes_ptr && sqes_ptr != MAP_FAILED) munmap(sqes_ptr, params.sq_entries * sizeof(struct io_uring_sqe));
    if (ring_fd >= 0) close(ring_fd);
  }
};

int main() {
  const char* path = "/tmp/uring_rw_probe.dat";
  int file_fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (file_fd < 0) { std::perror("open"); return 1; }

  Uring u;
  if (!u.setup(8)) { std::perror("uring setup"); return 1; }

  const char* payload = "hello-from-io-uring";
  size_t plen = std::strlen(payload);

  // WRITE
  struct io_uring_sqe wreq{};
  wreq.opcode = IORING_OP_WRITE;
  wreq.fd = file_fd;
  wreq.addr = reinterpret_cast<uint64_t>(payload);
  wreq.len = static_cast<uint32_t>(plen);
  wreq.user_data = 1;

  struct io_uring_cqe wcqe{};
  if (!u.submit_and_wait(wreq, &wcqe) || wcqe.res != static_cast<int>(plen)) {
    std::fprintf(stderr, "write failed res=%d\n", wcqe.res);
    return 1;
  }

  // READ back
  char rbuf[64] = {};
  struct io_uring_sqe rreq{};
  rreq.opcode = IORING_OP_READ;
  rreq.fd = file_fd;
  rreq.addr = reinterpret_cast<uint64_t>(rbuf);
  rreq.len = sizeof(rbuf) - 1;
  rreq.off = 0;
  rreq.user_data = 2;

  struct io_uring_cqe rcqe{};
  if (!u.submit_and_wait(rreq, &rcqe)) {
    std::fprintf(stderr, "read submit failed\n");
    return 1;
  }

  bool match = (rcqe.res == static_cast<int>(plen) && std::strcmp(rbuf, payload) == 0);

  std::printf("{\n");
  std::printf("  \"probe\": \"io-uring-real-rw\",\n");
  std::printf("  \"write_res\": %d,\n", wcqe.res);
  std::printf("  \"read_res\": %d,\n", rcqe.res);
  std::printf("  \"data_match\": %s,\n", match ? "true" : "false");
  std::printf("  \"payload\": \"%s\",\n", rbuf);
  std::printf("  \"note\": \"real io_uring file write + read via syscall + mmap\"\n");
  std::printf("}\n");

  close(file_fd);
  unlink(path);
  return match ? 0 : 1;
}
