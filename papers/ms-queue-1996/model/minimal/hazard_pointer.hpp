// Minimal hazard pointer domain (toy).
// Protects nodes from being freed while any thread holds a hazard pointer to them.

#pragma once
#include <atomic>
#include <vector>

constexpr int kHPSlots = 2;

struct HPRecord {
  std::atomic<void*> slots[kHPSlots]{};
  std::atomic<HPRecord*> next{nullptr};
  std::vector<void*> retired;
};

class HazardPointerDomain {
public:
  static HazardPointerDomain& instance() {
    static HazardPointerDomain d;
    return d;
  }

  HPRecord* acquire() {
    thread_local HPRecord* mine = nullptr;
    if (mine) return mine;
    mine = new HPRecord();
    HPRecord* old = head_.load(std::memory_order_relaxed);
    do {
      mine->next.store(old, std::memory_order_relaxed);
    } while (!head_.compare_exchange_weak(old, mine,
                                          std::memory_order_release,
                                          std::memory_order_relaxed));
    return mine;
  }

  void retire(void* p, void (*deleter)(void*)) {
    HPRecord* rec = acquire();
    rec->retired.push_back(p);
    if (rec->retired.size() >= 16) scan(rec, deleter);
  }

  void scan(HPRecord* rec, void (*deleter)(void*)) {
    std::vector<void*> protected_ptrs;
    for (HPRecord* r = head_.load(std::memory_order_acquire); r;
         r = r->next.load(std::memory_order_acquire)) {
      for (int i = 0; i < kHPSlots; ++i) {
        if (void* h = r->slots[i].load(std::memory_order_acquire))
          protected_ptrs.push_back(h);
      }
    }
    std::vector<void*> keep;
    for (void* node : rec->retired) {
      bool is_protected = false;
      for (void* h : protected_ptrs) {
        if (h == node) { is_protected = true; break; }
      }
      if (!is_protected) deleter(node);
      else keep.push_back(node);
    }
    rec->retired.swap(keep);
  }

private:
  std::atomic<HPRecord*> head_{nullptr};
};
