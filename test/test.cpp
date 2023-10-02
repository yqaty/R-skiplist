#include <bits/stdc++.h>

#include "client.h"

int main() {
  srand(time(0));
  Client client{3, 1.0 / 3, 3, BUFSIZE};
  for (int i = 10; i; --i) {
    client.insert(Node(i, i));
  }
  client.print();
  for (int i = 1; i <= 10; ++i) {
    printf("%d:\n", i);
    client.search(i)->print();
  }
  client.update(Node(5, 250));
  std::vector<Node> nodes = client.range_query(2, 8);
  for (auto u : nodes) {
    u.print();
  }
  return 0;
}