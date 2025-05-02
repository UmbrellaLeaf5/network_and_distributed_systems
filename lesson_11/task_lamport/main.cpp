#include "queue.hpp"

int main() {
  LamportQueue lq(8);

  lq.Add({1, 0});
  lq.Add({1, 1});
  lq.Add({1, 2});

  lq.Add({2, 5});
  lq.Add({2, 4});
  lq.Add({2, 3});

  lq.Add({0, 6});
  lq.Add({1, 7});

  std::cout << lq << "\n";

  lq.Remove(5);
  lq.Remove(1);
  lq.Remove(7);

  std::cout << lq << "\n";

  lq.Add({0, 5});
  lq.Add({3, 7});
  lq.Add({4, 1});

  std::cout << lq << "\n";

  lq.Add({4, 1});
  lq.Add({4, 8});

  return 0;
}
