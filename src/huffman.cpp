#include <iostream>
#include <cstddef>
#include <vector>
#include <set>
#include <string>
#include <iterator>
#include <algorithm>
#include <array>
#include <cassert>


#include "huffman.h"

using namespace std;

uint8_t to_code(char c) {
  return uint8_t(c - '\0');
}

unsigned char to_char(int i) {
  return reinterpret_cast<unsigned char>(uint8_t(i));
}

TreeNode::TreeNode(size_t cnt, unsigned char letter) :
cnt_(cnt), letter_(letter), left_(nullptr), right_(nullptr) {};

TreeNode::TreeNode(size_t cnt, shared_ptr<TreeNode> left,
  shared_ptr<TreeNode> right) : cnt_(cnt), left_(left), right_(right) {};

shared_ptr<TreeNode> HuffTree::get_root(){
  return root;
};

vector<bool> HuffTree::get_code(unsigned char letter){
  return final_codes[letter];
};

struct Comparator {
  bool operator()(shared_ptr<TreeNode> const& n1, shared_ptr<TreeNode> const& n2) {
    return (n1->cnt_ < n2->cnt_) || ((n1->cnt_ == n2->cnt_) && (to_code(n1->letter_) < to_code(n2->letter_)));
  }
};

void HuffTree::set_codes(shared_ptr<TreeNode> node, vector<bool> current_code) {
  if(node->left_ == nullptr) {
    final_codes[node->letter_] = current_code;
    return;
  }

  current_code.push_back(0);
  set_codes(node->left_, current_code);
  current_code.pop_back();

  current_code.push_back(1);
  set_codes(node->right_, current_code);
  current_code.pop_back();
}

array<size_t, 256> exctract_counts(istream& input) {
  array<size_t, 256> table = {};
  char c;

  while (input.read(&c, 1)) {
    uint8_t code = to_code(c);
    table[code]++;
  }

  input.clear();
  return table;
};

HuffTree::HuffTree(array<size_t, 256> table) {

  multiset<shared_ptr<TreeNode>, Comparator> nodes;
  for(int i = 0; i < 256; i++) {
    if(table[i] > 0) {
      unsigned char c = to_char(i);
      nodes.insert(make_shared<TreeNode>(table[i], c));
    }
  }

  if(nodes.size() == 0) {
    return;
  }

  while(nodes.size() > 1) {
    shared_ptr<TreeNode> first_min = *nodes.begin();
    nodes.erase(nodes.begin());

    shared_ptr<TreeNode> second_min = *nodes.begin();
    nodes.erase(nodes.begin());

    TreeNode cur_node = TreeNode(first_min->cnt_ + second_min->cnt_,
      first_min, second_min);
    nodes.insert(make_shared<TreeNode>(cur_node));
  }

  root = *nodes.begin();
  vector<bool> current_code;
  if(root->left_ == nullptr) {
    current_code.push_back(0);
  }
  set_codes(root, current_code);
}

size_t get_input_size(istream& input) {
  streampos fsize = 0;
  input.seekg(0, ios::end);
  fsize = input.tellg() - fsize;
  input.seekg(0, ios::beg);
  return (size_t) fsize;
}

size_t get_output_size(ostream& output) {
  streampos fsize = 0;
  output.seekp(0, ios::end);
  fsize = output.tellp() - fsize;
  output.seekp(0, ios::beg);
  return (size_t) fsize;
}

HuffArchiver::HuffArchiver(istream& input1, ostream& output1):
input(input1), output(output1){};

BinaryWriter::BinaryWriter(ostream& output) :
byte(0), pos(0), stream(output) {};

void BinaryWriter::write(char const* buffer, size_t cnt) {
  assert(pos == 0);
  stream.write(buffer, cnt);
}

void BinaryWriter::put(char c) {
  assert(pos == 0);
  stream.put(c);
}

void BinaryWriter::write_bit(bool bit) {
  byte ^= (bit << pos);
  pos++;
  if(pos == 8) {
    shift_byte();
  }
}

void BinaryWriter::shift_byte() {
  if(pos > 0){
    stream.write(&byte, sizeof(char));
    pos = 0;
    byte = 0;
  }
}

BinaryReader::BinaryReader(istream& input) :
byte(0), pos(0), stream(input) {};

void BinaryReader::read(char* buffer, size_t cnt) {
  assert(pos == 0);
  stream.read(buffer, cnt);
}

bool BinaryReader::read_bit() {
  if(pos == 0) {
    read(&byte, 1);
  }

  bool ans = (byte & (1 << pos));
  pos++;
  pos %= 8;

  return ans;
}

CompressionInfo HuffArchiver::zip() {
  CompressionInfo result;

  array<size_t, 256> table = exctract_counts(input);

  result.original = get_input_size(input);

  size_t nonzero = 0;
  for(int i = 0; i < 256; i++) {
    if(table[i] > 0) {
      nonzero++;
    }
  }

  if(nonzero == 0) {
    result.original = 0;
    result.compressed = 0;
    result.additional = 0;
    return result;
  }

  result.additional = sizeof(int) + (sizeof(char) + sizeof(int)) * nonzero;

  BinaryWriter output_writer = BinaryWriter(output);

  output_writer.write((const char*)&nonzero, sizeof(int));

  for(int i = 0; i < 256; i++) {
    if(table[i] > 0) {
      unsigned char c = reinterpret_cast<unsigned char>((uint8_t) i);
      output_writer.put(c);
      output_writer.write((const char*) &table[i], sizeof(int));
    }
  }

  HuffTree tree = HuffTree(table);

  unsigned char c;
  while (input.read((char*)&c, 1)) {
    vector<bool> code = tree.get_code(c);
    for (size_t i = 0; i < code.size(); i++) {
        output_writer.write_bit(code[i]);
    }
  }
  output_writer.shift_byte();

  result.compressed = get_output_size(output) - result.additional;
  return result;
}

CompressionInfo HuffArchiver::unzip() {
  CompressionInfo result;

  if(get_input_size(input) == 0) {
    result.original = 0;
    result.compressed = 0;
    result.additional = 0;
    return result;
  }

  array<size_t, 256> table = {};

  BinaryReader input_reader = BinaryReader(input);

  int nonzero;
  input.read((char*)&nonzero, sizeof(int));

  int total_length = 0;
  for(int i = 0; i < nonzero; i++) {
    unsigned char c;
    int cnt;
    input.read((char*)&c, sizeof(char));
    input.read((char*)&cnt, sizeof(int));
    uint8_t code = to_code(c);
    table[code] = cnt;
    total_length += cnt;
  }
  result.additional = sizeof(int) + (sizeof(char) + sizeof(int)) * nonzero;

  HuffTree tree = HuffTree(table);

  for(int i = 0; i < total_length; i++) {
    shared_ptr<TreeNode> node = tree.get_root();
    while(node->left_ != nullptr) {
      bool byte = input_reader.read_bit();
      if (byte) {
        node = node->right_;
      } else {
        node = node->left_;
      }
    }
    output.write(reinterpret_cast<const char*> (&(node->letter_)), sizeof(char));
  }

  input.clear();
  result.original = get_input_size(input) - result.additional;
  result.compressed = get_output_size(output);
  return result;
};
