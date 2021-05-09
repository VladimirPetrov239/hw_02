#ifndef LAB_13_MY_ARRAY_H
#define LAB_13_MY_ARRAY_H

#include <cstdio>
#include <cstddef>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <array>
#include <memory>


uint8_t to_code(char c);

unsigned char to_char(int i);

class TreeNode {
public:
  std::size_t cnt_;
  unsigned char letter_ = '\0';
  std::shared_ptr<TreeNode> left_;
  std::shared_ptr<TreeNode> right_;

  TreeNode(std::size_t cnt, unsigned char letter);
  TreeNode(std::size_t cnt, std::shared_ptr<TreeNode> left, std::shared_ptr<TreeNode> right);
};

class HuffTree {
public:
    HuffTree(std::array<size_t, 256> table);

    std::shared_ptr<TreeNode> get_root();
    std::vector<bool> get_code(unsigned char letter);

private:
    std::shared_ptr<TreeNode> root;
    std::array<std::vector<bool>, 256> final_codes;
    void set_codes(std::shared_ptr<TreeNode> node, std::vector<bool> current_code);
};

std::array<size_t, 256> exctract_counts(std::istream& input);

class BinaryWriter {
public:
  BinaryWriter(std::ostream& output);

  void write(char const* buffer, std::size_t cnt);
  void put(char c);
  void write_bit(bool bit);
  void shift_byte();

private:
  char byte;
  std::size_t pos;
  std::ostream& stream;
};

class BinaryReader {
public:
  BinaryReader(std::istream& input);

  void read(char* buffer, std::size_t cnt);
  bool read_bit();

private:
  char byte;
  std::size_t pos;
  std::istream& stream;
};

struct CompressionInfo {
    std::size_t original;
    std::size_t additional;
    std::size_t compressed;
};

class HuffArchiver {

public:
  HuffArchiver(std::istream& input, std::ostream& output);

  CompressionInfo zip();
  CompressionInfo unzip();

private:
    std::istream& input;
    std::ostream& output;
};

#endif
