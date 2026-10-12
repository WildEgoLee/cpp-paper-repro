// Minimal Copy-on-Write page.
// Shared while read-only; clone on write if refcount > 1.

#pragma once
#include <memory>
#include <string>

struct Page {
  std::string data;
  int refcount = 1;

  explicit Page(std::string d) : data(std::move(d)) {}
};

class CoWPage {
public:
  explicit CoWPage(std::shared_ptr<Page> p) : page_(std::move(p)) {
    if (page_) ++page_->refcount;
  }

  CoWPage(const CoWPage& other) : page_(other.page_) {
    if (page_) ++page_->refcount;
  }

  CoWPage& operator=(const CoWPage& other) {
    if (this != &other) {
      release();
      page_ = other.page_;
      if (page_) ++page_->refcount;
    }
    return *this;
  }

  ~CoWPage() { release(); }

  const std::string& read() const { return page_->data; }

  void write(const std::string& new_data) {
    if (page_->refcount > 1) {
      // Shared: clone
      --page_->refcount;
      page_ = std::make_shared<Page>(new_data);
      page_->refcount = 1;
    } else {
      // Exclusive: mutate in place
      page_->data = new_data;
    }
  }

  int refcount() const { return page_ ? page_->refcount : 0; }

private:
  void release() {
    if (page_) {
      --page_->refcount;
      if (page_->refcount == 0) {
        // shared_ptr will delete when last owner drops
      }
      page_.reset();
    }
  }

  std::shared_ptr<Page> page_;
};
