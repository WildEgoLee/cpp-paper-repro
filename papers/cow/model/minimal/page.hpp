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
  explicit CoWPage(std::shared_ptr<Page> p);

  // Read: no copy
  const std::string& read() const;

  // Write: copy if shared
  void write(const std::string& new_data);

  int refcount() const;

private:
  std::shared_ptr<Page> page_;
};
