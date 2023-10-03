#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "client.h"
int main() {
  srand(time(0));
  Client client{3, 1.0 / 3, 3, BUFSIZE};
  for (int i = 10; i; --i) {
    client.insert(Node(i, i));
  }
  client.print();
  for (int i = 1; i <= 11; ++i) {
    printf("search %d:\n", i);
    Node* node = client.search(i);
    if (node != nullptr) {
      node->print();
    } else {
      printf("%d not found!\n", i);
    }
  }
  client.update(Node(5, 1212));
  std::vector<Node> nodes = client.range_query(2, 8);
  for (auto u : nodes) {
    u.print();
  }
  for (int i = 1; i <= 10; ++i) {
    client.remove(i);
  }
  client.print();
  return 0;
}