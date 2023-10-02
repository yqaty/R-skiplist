#ifndef CLIENT_H
#define CLIENT_H

#include <bits/stdc++.h>

const int BUFSIZE = 1 << 22;
class Alloc {
  char *buffer;
  int size;
  int now;

 public:
  Alloc(int _size);
  ~Alloc();
  void *alloc(int size);
};

struct Block {
  int cap;
  int len;
  int size;
  Block *next;
  bool is_update;
  char data[0];

 public:
  void print();
  void init(int _cap, int _size);
  int get_key();
  bool is_empty();
  bool is_full();
} __attribute__((aligned(1)));

struct Node {
  int key;
  int value;
  Block *down;
  bool is_delete;
  bool is_head;
  void print();
  Node(int _key, int _value);
} __attribute__((aligned(1)));

class Client {
  Alloc alloc;
  int level;
  int cap;
  double p;
  Block *head;

 public:
  Client(int _level, double _p, int _cap, int bufsize);
  void print();
  Block *init_block(int _cap);
  int get_height();
  void add_node(Block *block, const Node &node);
  int get_node_from_block(Block *block, int key);
  bool get_range_nodes_from_block(std::vector<Node> &nodes, Block *block,
                                  int st, int ed);
  void insert_block(Block *block, const Node &node);
  Block *split_block_by_key(Block *block, int key);
  Block *split_block_equally(Block *block);
  void remove_node(Block *block, int key);
  void remove_block(Block *block);
  Block *remote_get(Block *address);
  Block *get_block(Block *address);
  void find(int key, Block *target_blocks[], Node *target_nodes[]);
  void insert(const Node &node);
  void remove(int key);
  Node *search(int key);
  void update(const Node &node);
  std::vector<Node> range_query(int st, int ed);
};

#endif