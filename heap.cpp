#include <cctype>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <numeric>
#include <stack>
#include <vector>

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
  os << "[ ";
  for (const auto& x : v) os << x << ' ';
  os << "]";
  return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::stack<T>& s) {
  std::stack<T> tmp(s);
  os << "[ ";
  while (!tmp.empty()) {
    os << tmp.top() << ' ';
    tmp.pop();
  }
  os << "]";
  return os;
}

template <typename T>
class Heap {
 public:
  // Assumes T has a size constructor
  Heap(std::size_t n) : heap_(std::make_unique<T>(n + offset_)), size_(n) {}
  Heap(const std::vector<T>& v)
      : heap_(std::make_unique<T[]>(v.size())), size_(v.size()) {
    for (std::size_t i = 0; i < v.size(); ++i) {
      heap_[offset_ + i] = v[i];
    }
    heapify();
  }
  Heap(const std::vector<T>& v, std::size_t offset)
      : heap_(std::make_unique<T[]>(v.size() + offset)),
        offset_(offset),
        size_(v.size()) {
    for (std::size_t i = 0; i < v.size(); ++i) {
      heap_[offset_ + i] = v[i];
    }
    heapify();
  }

  ~Heap() { heap_.release(); }

  std::size_t size() { return size_; }

  T Top() { return heap_[offset_]; }

  void set_offset(std::size_t offset) {
    offset_ = offset;
    heapify();
  }

  friend std::ostream& operator<<(std::ostream& os, const Heap& h) {
    os << "[ ";
    for (std::size_t i = 0; i < h.size_ + h.offset_; ++i) {
      os << h.heap_[i] << ' ';
    }
    os << "]";
    return os;
  }

 private:
  void heapify();

  void swap_idx(std::size_t idx1, std::size_t idx2) {
    auto tmp = heap_[idx1];
    heap_[idx1] = heap_[idx2];
    heap_[idx2] = tmp;
  }

  std::size_t last_parent() {
    return static_cast<std::size_t>(std::log2(size_ + 1));
  }
  std::size_t lchild_index(std::size_t idx) { return (idx - offset_) * 2 + 1 + offset_; }
  std::size_t rchild_index(std::size_t idx) { return (idx - offset_) * 2 + 2 + offset_; }

  // Member variables
  std::unique_ptr<T[]> heap_;
  std::size_t size_;
  std::size_t offset_ = 0;

};  // class Heap

template <typename T>
void Heap<T>::heapify() {
  std::stack<std::size_t> index_stack;
  index_stack.push(static_cast<std::size_t>(offset_ + size_));
  while (!index_stack.empty()) {
    const auto parent_idx = index_stack.top();
    index_stack.pop();

    if (parent_idx > offset_) {
      index_stack.push(parent_idx - 1);
    }
    if (lchild_index(parent_idx) > offset_ + size_ - 1 && rchild_index(parent_idx) > offset_ + size_ - 1) continue;

    auto lchild = heap_[lchild_index(parent_idx)];
    auto rchild = heap_[rchild_index(parent_idx)];

    if (lchild_index(parent_idx) < offset_ + size_ && lchild < heap_[parent_idx]) {
      swap_idx(lchild_index(parent_idx), parent_idx);
      index_stack.push(lchild_index(parent_idx));
    }
    if (rchild_index(parent_idx) < offset_ + size_ && rchild < heap_[parent_idx]) {
      swap_idx(rchild_index(parent_idx), parent_idx);
      index_stack.push(rchild_index(parent_idx));
    }
  }
}

int main() {
  using steady_clock = std::chrono::steady_clock;

  std::cout << "#######\n"
            << " START\n"
            << "######\n";
  constexpr int N = 5000;
  constexpr int N_TRIALS = 50;
  using value_type = float;
  std::vector<value_type> v;
  for (int i = N; i > 0; --i) v.push_back(static_cast<value_type>(i));
  std::cout << "Warming up cache...\n";
  for (int i = 0; i < N_TRIALS; ++i) volatile Heap<value_type> h(v);

  std::cout << "Building heap\n";

  std::vector<double> standard_offset(N_TRIALS);

  for (int i = 0; i < N_TRIALS; ++i) {
    auto start = steady_clock::now();
    Heap<value_type> h(v, 0);
    auto stop = steady_clock::now();

    std::chrono::duration<double> elapsed = stop - start;
    standard_offset[i] = elapsed.count();
  }

  double standard_time = std::accumulate(standard_offset.begin(), standard_offset.end(), 0.0)/N_TRIALS;
  std::cout << "Averaged over " << N_TRIALS << " runs\n";
  std::cout << "\tStandard layout heap built in " << standard_time << " seconds\n";

  for (int i = 0; i < 10; ++i) {
    std::vector<double> h_offset(N_TRIALS);
    for (int j = 0; j < N_TRIALS; ++j) {
      // Time heap construction
      auto start = steady_clock::now();
      Heap<value_type> ho(v, i);
      auto stop = steady_clock::now();

      std::chrono::duration<double> elapsed = stop - start;
      h_offset[j] = elapsed.count();
    }
    double time = std::accumulate(h_offset.begin(), h_offset.end(), 0.0)/N_TRIALS;
    std::cout << "\tHeap offset used: " << i << " heap built in " << time << " seconds (" << time / standard_time << "\% of standard)\n";

  }

  return 0;
}
