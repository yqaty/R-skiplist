#include "client.h"

Alloc::Alloc(int _size) : size(_size) {
  now = 0;
  buffer = (char*)malloc(size);
  memset(buffer, 0, size);
}

Alloc::~Alloc() { free(buffer); }

void* Alloc::alloc(int _size) {
  if (now + _size >= size) {
    return nullptr;
  }
  now += _size;
  return buffer + now - _size;
}

void Block::print() {
  printf("block: cap=%d len=%d\n", cap, len);
  Node* node = (Node*)data;
  for (int i = 0; i < len; ++i) {
    (node + i)->print();
  }
}

void Block::init(int _cap, int _size) {
  cap = _cap;
  size = _size;
  is_update = false;
  len = 0;
}

int Block::get_key() {
  Node* node = (Node*)data;
  return node->key;
}

bool Block::is_empty() { return len == 0; }

bool Block::is_full() { return cap == len; }

void Node::print() {
  printf("node: key=%d value=%d ", key, value);
  if (is_delete) printf("is_delete");
  puts("");
}

Node::Node(int _key, int _value)
    : key(_key), value(_value), is_delete(false), is_head(false) {}

void Client::print() {
  Block* now = head;
  printf("level = %d, p = %f\n", level, p);
  for (int i = level - 1; i >= 0; --i) {
    puts("---------------------------------------");
    printf("layer %d\n", i);
    Block* block = now;
    while (!block->is_empty()) {
      block->print();
      block = block->next;
    }
    if (i) {
      now = ((Node*)now->data)->down;
    }
    puts("---------------------------------------");
  }
}

Client ::Client(int _level, double _p, int _cap, int bufsize)
    : level(_level), p(_p), cap(_cap), alloc(bufsize) {
  Block* block[level];
  for (int i = 0; i < level; ++i) {
    block[i] = init_block(i == level - 1 ? 1 : cap + 1);
    if (i) {
      ((Node*)block[i]->data)->down = block[i - 1];
    }
    ((Node*)block[i]->data)->is_head = true;
    block[i]->len = 1;
    block[i]->next = init_block(0);
  }
  head = block[level - 1];
}

Block* Client::init_block(int _cap) {
  Block* block = (Block*)alloc.alloc(sizeof(Block) + _cap * sizeof(Node));
  block->init(_cap, sizeof(Node));
  return block;
}

int Client::get_height() {
  int h = 1;
  while (h < level && rand() <= p * RAND_MAX) {
    h++;
  }
  return h;
}

void Client::add_node(Block* block, const Node& node) {
  if (block->is_full()) {
    return;
  }
  Node* block_node = (Node*)block->data;
  memcpy(block_node + (block->len), &node, sizeof(Node));
  block->len++;
}

int Client::get_node_from_block(Block* block, int key) {
  Node* block_node = (Node*)block->data;
  int pos = 0;
  while (pos + 1 < block->len && (block_node + pos + 1)->key <= key) {
    pos++;
  }
  return pos;
}

bool Client::get_range_nodes_from_block(std::vector<Node>& nodes, Block* block,
                                        int st, int ed) {
  if (block->is_empty()) {
    return false;
  }
  Node* block_node = (Node*)block->data;
  for (int i = block_node->is_head ? 1 : 0; i < block->len; ++i) {
    if ((block_node + i)->key >= st && (block_node + i)->key <= ed &&
        !(block_node + i)->is_delete) {
      nodes.push_back(*(block_node + i));
    }
  }
  return (block_node + block->len - 1)->key <= ed;
}

void Client::insert_block(Block* block, const Node& node) {
  Node* block_node = (Node*)block->data;
  int pos = get_node_from_block(block, node.key);
  for (int i = block->len; i > pos + 1; --i) {
    memcpy(block_node + i, block_node + i - 1, sizeof(Node));
  }
  memcpy(block_node + pos + 1, &node, sizeof(Node));
  block->len++;
}

Block* Client::split_block_by_key(Block* block, int key) {
  Node* block_node = (Node*)block->data;
  int pos = get_node_from_block(block, key);
  Block* new_block = init_block(cap + 1);
  for (int i = pos; i < block->len; ++i) {
    add_node(new_block, *(block_node + i));
  }
  block->len = pos;
  new_block->next = block->next;
  block->next = new_block;
  return new_block;
}

Block* Client::split_block_equally(Block* block) {
  Node* block_node = (Node*)block->data;
  int pos = block->cap / 2;
  Block* new_block = init_block(cap + 1);
  for (int i = pos; i < block->len; ++i) {
    add_node(new_block, *(block_node + i));
  }
  block->len = pos;
  new_block->next = block->next;
  block->next = new_block;
  return new_block;
}

void Client::remove_node(Block* block, int key) {
  Node* block_node = (Node*)block->data;
  int pos = get_node_from_block(block, key);
  for (int i = pos + 1; i < block->len; ++i) {
    memcpy(block_node + i - 1, block_node + i, sizeof(Node));
  }
  block->len--;
}

Block* Client::remote_get(Block* address) { return address; }

Block* Client::get_block(Block* address) {
  Block* block = remote_get(address);
  while (block->is_update) {
    block = remote_get(address);
  }
  return block;
}

void Client::find(int key, Block* target_blocks[], Node* target_nodes[]) {
  Block* now = head;
  for (int i = level - 1; i >= 0; --i) {
    Block* next = now->next;
    while (!next->is_empty() && next->get_key() <= key) {
      now = next;
      next = now->next;
    }
    target_blocks[i] = now;
    Node* node = (Node*)now->data;
    int pos = get_node_from_block(now, key);
    target_nodes[i] = node + pos;
    if (i) now = node->down;
  }
}

void Client::insert(const Node& _node) {
  Node node = _node;
  Block* target_blocks[level];
  Node* target_nodes[level];
  find(node.key, target_blocks, target_nodes);
  if (!target_nodes[0]->is_head && target_nodes[0]->key == node.key) {
    if (target_nodes[0]->is_delete) {
      update(_node, true);
    }
    return;
  }
  int h = get_height();
  for (int i = 0; i < h; ++i) {
    target_blocks[i]->is_update = true;
    if (i == level - 1) {
      Block* new_block = init_block(1);
      new_block->next = target_blocks[i]->next;
      target_blocks[i]->next = new_block;
      add_node(new_block, node);
    } else {
      insert_block(target_blocks[i], node);
      if (i + 1 < h) {
        node.down = split_block_by_key(target_blocks[i], node.key);
      } else if (target_blocks[i]->is_full()) {
        split_block_equally(target_blocks[i]);
      }
    }

    target_blocks[i]->is_update = false;
  }
}

void Client::remove(int key) {
  Block *target_blocks[level], *pre_blocks[level];
  Node *target_nodes[level], *pre_nodes[level];
  find(key, target_blocks, target_nodes);
  find(key - 1, pre_blocks, pre_nodes);
  bool is_mark = false;
  if (!target_nodes[level - 1]->is_head &&
      target_nodes[level - 1]->key == key) {
    is_mark = true;
  }
  for (int i = level - 1; i >= 0; i--) {
    if (!target_nodes[i]->is_head && target_nodes[i]->key == key) {
      if (is_mark) {
        target_nodes[i]->is_delete = true;
      } else {
        target_blocks[i]->is_update = true;
        remove_node(target_blocks[i], key);
        if (target_blocks[i]->is_empty()) {
          target_blocks[i]->is_update = true;
          pre_blocks[i]->next = target_blocks[i]->next;
          target_blocks[i]->is_update = false;
        }
        target_blocks[i]->is_update = false;
      }
    }
  }
}

Node* Client::search(int key) {
  Block* address = head;
  Block* block;
  Node* node;
  for (int i = level - 1; i >= 0; i--) {
    block = get_block(address);
    while (true) {
      Block* next_block = get_block(block->next);
      if (!next_block->is_empty() && next_block->get_key() <= key) {
        block = next_block;
      } else {
        break;
      }
    }
    int pos = get_node_from_block(block, key);
    node = (Node*)(block->data + sizeof(Node) * pos);
    if (i) {
      address = node->down;
    }
  }
  if (node->key == key && !node->is_delete) {
    Node* res = (Node*)malloc(sizeof(Node));
    memcpy(res, node, sizeof(Node));
    return res;
  } else {
    return nullptr;
  }
}

void Client::update(const Node& node, bool op) {
  Block* target_blocks[level];
  Node* target_nodes[level];
  find(node.key, target_blocks, target_nodes);
  if (target_nodes[0]->is_head || target_nodes[0]->key != node.key ||
      (target_nodes[0]->is_delete && !op)) {
    return;
  }
  for (int i = 0; i < level; ++i) {
    if (!target_nodes[i]->is_head && target_nodes[i]->key == node.key) {
      target_nodes[i]->value = node.value;
    }
  }
}

std::vector<Node> Client::range_query(int st, int ed) {
  Block* address = head;
  Block* block;
  Node* node;
  for (int i = level - 1; i >= 0; i--) {
    block = get_block(address);
    while (true) {
      Block* next_block = get_block(block->next);
      if (!next_block->is_empty() && next_block->get_key() <= st) {
        block = next_block;
      } else {
        break;
      }
    }
    int pos = get_node_from_block(block, st);
    node = (Node*)(block->data + sizeof(Node) * pos);
    if (i) {
      address = node->down;
    }
  }
  std::vector<Node> nodes;

  while (get_range_nodes_from_block(nodes, block, st, ed)) {
    block = get_block(block->next);
  }
  return nodes;
}