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
  std::size_t lchild_index(std::size_t idx) { return idx * 2 + 1; }
  std::size_t rchild_index(std::size_t idx) { return idx * 2 + 2; }

  // Member variables
  std::unique_ptr<T[]> heap_;
  std::size_t size_;
  std::size_t offset_ = 0;

};  // class Heap

template <typename T>
void Heap<T>::heapify() {
  std::stack<std::size_t> index_stack;
  index_stack.push(static_cast<std::size_t>((offset_ + size_) / 2));
  while (!index_stack.empty()) {
    const auto parent_idx = index_stack.top();
    index_stack.pop();

    if (parent_idx > offset_) {
      index_stack.push(parent_idx - 1);
    }

    auto lchild = heap_[lchild_index(parent_idx)];
    auto rchild = heap_[rchild_index(parent_idx)];
    if (lchild_index(parent_idx) < size_ && lchild < heap_[offset_ + parent_idx]) {
      swap_idx( lchild_index(parent_idx), parent_idx);
      index_stack.push(lchild_index(parent_idx));
    }
    if (rchild_index(parent_idx) < size_ && rchild < heap_[parent_idx]) {
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
  constexpr int N = 10;
  constexpr int N_TRIALS = 1;
  using value_type = short;
  std::vector<value_type> v;
  for (int i = N; i > 0; --i) v.push_back(static_cast<value_type>(i));
  std::cout << "Building heap\n";

  std::vector<double> standard_offset(N_TRIALS);
  std::vector<double> h1_offset(N_TRIALS);
  std::vector<double> h2_offset(N_TRIALS);
  std::vector<double> h3_offset(N_TRIALS);
  std::vector<double> h4_offset(N_TRIALS);

  for (int i = 0; i < N_TRIALS; ++i) {
    auto start = steady_clock::now();
    Heap<value_type> h(v);
    auto stop = steady_clock::now();
    std::chrono::duration<double> elapsed = stop - start;
    standard_offset[i] = elapsed.count();

    start = steady_clock::now();
    Heap<value_type> h1(v, 1);
    std::cout << "h1: " << h1 << '\n';
    stop = steady_clock::now();
    elapsed = stop - start;
    h1_offset[i] = elapsed.count();

    start = steady_clock::now();
    Heap<value_type> h2(v, 2);
    std::cout << "h2: " << h2 << '\n';
    stop = steady_clock::now();
    elapsed = stop - start;
    h2_offset[i] = elapsed.count();

    start = steady_clock::now();
    Heap<value_type> h3(v, 3);
    std::cout << "h3: " << h3 << '\n';

    stop = steady_clock::now();
    elapsed = stop - start;
    h3_offset[i] = elapsed.count();

    start = steady_clock::now();
    Heap<value_type> h4(v, 4);
    std::cout << "h4: " << h4 << '\n';
    stop = steady_clock::now();
    elapsed = stop - start;
    h4_offset[i] = elapsed.count();
  }

  double standard_time = std::accumulate(standard_offset.begin(), standard_offset.end(), 0.0)/standard_offset.size();
  double h1_time = std::accumulate(h1_offset.begin(), h1_offset.end(), 0.0)/h1_offset.size();
  double h2_time = std::accumulate(h2_offset.begin(), h2_offset.end(), 0.0)/h2_offset.size();
  double h3_time = std::accumulate(h3_offset.begin(), h3_offset.end(), 0.0)/h3_offset.size();
  double h4_time = std::accumulate(h4_offset.begin(), h4_offset.end(), 0.0)/h4_offset.size();

  std::cout << "Average over " << N_TRIALS << " runs:"
            << "\n\tStandard layout heap built in " << standard_time
            << "\n\tH1 offset heap built in " << h1_time << " (" << h1_time / standard_time << "\% of standard)"
            << "\n\tH1 offset heap built in " << h2_time << " (" << h2_time / standard_time << "\% of standard)"
            << "\n\tH1 offset heap built in " << h3_time << " (" << h3_time / standard_time << "\% of standard)"
            << "\n\tH1 offset heap built in " << h4_time << " (" << h4_time / standard_time << "\% of standard)"
            << "\n";

  return 0;
}
