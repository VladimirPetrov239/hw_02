#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "huffman.h"

#include <climits>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <cstring>

using namespace std;

TEST_CASE("test #1 of the table function") {
    stringstream in;
    in << "My Name Is Slim Shady";

    array<size_t, 256> table = exctract_counts(in);
    CHECK(table['S'] == 2);
    CHECK(table['s'] == 1);
    CHECK(table['m'] == 2);
    CHECK(table[' '] == 4);
}

TEST_CASE("test #2 of the table function") {
  stringstream in;
  in << "";

  array<size_t, 256> table = exctract_counts(in);
  CHECK(table[to_code('f')] == 0);
  CHECK(table[to_code('k')] == 0);
  CHECK(table['.'] == 0);
  CHECK(table['$'] == 0);
  CHECK(table['\n'] == 0);
}

TEST_CASE("test #1 of the get_code function") {
  stringstream in;
  in << "AAAAAbbbcd";

  array<size_t, 256> table = exctract_counts(in);
  HuffTree tree = HuffTree(table);

  CHECK(tree.get_code('A').size() == 1);
  CHECK(tree.get_code('b').size() == 2);
  CHECK(tree.get_code('c').size() == 3);
  CHECK(tree.get_code('d').size() == 3);
}

TEST_CASE("test #2 of the get_code function") {
  stringstream in;
  for(int i = 0; i < 239; i++) {
    in << '\0';
  }

  array<size_t, 256> table = exctract_counts(in);
  HuffTree tree = HuffTree(table);

  CHECK(tree.get_code('\0').size() == 1);
}

TEST_CASE("test #1 of the get_root function") {
  stringstream in;
  in << "AAAAAbbbcd";

  array<size_t, 256> table = exctract_counts(in);
  HuffTree tree = HuffTree(table);
  auto root = tree.get_root();

  CHECK(root->cnt_ == 10);
  CHECK(root->right_->cnt_ == 5);
  CHECK(root->left_->cnt_ == 5);
}

TEST_CASE("test #2 of the get_root function") {
  stringstream in;
  in << "$$$";

  array<size_t, 256> table = exctract_counts(in);
  HuffTree tree = HuffTree(table);
  auto root = tree.get_root();

  CHECK(root->cnt_ == 3);
  CHECK(root->letter_ == '$');
  CHECK(root->right_ == nullptr);
  CHECK(root->left_ == nullptr);
}

TEST_CASE("test of tree creation") {
  array<size_t, 256> table  = {};
  table[0] = 1;
  table[1] = 2;
  table[2] = 4;
  table[3] = 8;
  HuffTree tree = HuffTree(table);
  shared_ptr<TreeNode> root = tree.get_root();
  CHECK(root->cnt_ == 15);
  CHECK(root->left_->cnt_ == 7);
  CHECK(root->right_->cnt_ == 8);
}

TEST_CASE("test of the bin_write function") {
  stringstream s;
  BinaryWriter writer(s);
  writer.write("a", 1);

  char c;
  s >> c;
  CHECK(c == 'a');
}

TEST_CASE("test of the bin_write_bit function") {
  stringstream s;
  BinaryWriter writer(s);
  writer.write_bit(1);
  writer.write_bit(0);
  writer.write_bit(0);
  writer.write_bit(0);
  writer.write_bit(0);
  writer.write_bit(1);
  writer.write_bit(1);
  writer.write_bit(0);

  char c;
  s >> c;
  CHECK(c == 'a');
}

TEST_CASE("test of the bin_write_shift function") {
  stringstream s;
  BinaryWriter writer(s);
  writer.write_bit(0);
  writer.write_bit(0);
  writer.write_bit(0);
  writer.write_bit(0);
  writer.write_bit(0);
  writer.write_bit(0);
  writer.write_bit(1);
  writer.shift_byte();

  char c;
  s >> c;
  CHECK(c == '@');
}

TEST_CASE("test of the bin_read function") {
  stringstream s;
  s << "salam buleikum";
  BinaryReader reader(s);

  char* slovo = new char[6];
  reader.read(slovo, 5);
  slovo[5] = '\0';
  CHECK(std::string(slovo) == "salam");
  delete[] slovo;
}

TEST_CASE("test of the bin_read_bit function") {
  stringstream s("a");
  BinaryReader reader(s);

  CHECK(reader.read_bit() == 1);
  CHECK(reader.read_bit() == 0);
  CHECK(reader.read_bit() == 0);
  CHECK(reader.read_bit() == 0);
  CHECK(reader.read_bit() == 0);
  CHECK(reader.read_bit() == 1);
  CHECK(reader.read_bit() == 1);
  CHECK(reader.read_bit() == 0);
}

TEST_CASE("test of tree for 1-symbol string") {
  array<size_t, 256> table = {};
  table[0] = 239;
  HuffTree tree = HuffTree(table);
  CHECK(tree.get_root()->right_ == nullptr);
  CHECK(tree.get_root()->left_ == nullptr);
  CHECK(tree.get_root()->letter_ == '\0' );
}

TEST_CASE("ALL IN encoding test for 1-symbol code") {
  stringstream in, out, new_in;
  char c = '\0';
  in << c;

  HuffArchiver archiver = HuffArchiver(in, out);
  CompressionInfo result1 = archiver.zip();

  CHECK(result1.original == 1);
  CHECK(result1.compressed == 1);
  CHECK(result1.additional == 2 * sizeof(int) + sizeof(char));

  HuffArchiver unarchiver = HuffArchiver(out, new_in);
  CompressionInfo result2 = unarchiver.unzip();
  CHECK(result2.original == 1);
  CHECK(result2.compressed == 1);
}

TEST_CASE("Big Russian test for ALL IN encoding") {
  stringstream in, out, new_in;
  string s = R"LONG(Ха!
  Эй, цепь на мне, сыпь лавэ
  Сотка тыщ на bag LV
  Сотни сук хотят ко мне
  Сотни сук хотят камней
  Как дела? Как дела?
  Это новый Cadillac
  Делать деньги, делать деньги
  Делать деньги, блять, вот так (окей, few)
  Ay, bitch, we got some пушки (пр-р, пау)
  Пау-пау, попал по тушке (ха)
  На мне ща две подушки (оу да)
  Bitch, я висю, как молодой Пушкин (у)
  Цепи висят на папе (е)
  Копаем кэш лопатой (е)
  Богатый, будто каппер (е)
  Как там твоя зарплата?
  Эй, посмотри
  Два мульта на мне — часы
  Три на шее, семь под жопой
  Мне чуть больше двадцати
  Посмотри, посмотри
  Два мульта на мне — часы
  Три на шее, семь под жопой
  Мне чуть больше двадцати
  Эй, цепь на мне, сыпь лавэ
  Сотка тыщ на bag LV
  Сотни сук хотят ко мне
  Сотни сук хотят камней
  Как дела? Как дела?)LONG";
  in << s;

  HuffArchiver archiver = HuffArchiver(in, out);
  archiver.zip();
  HuffArchiver unarchiver = HuffArchiver(out, new_in);
  unarchiver.unzip();
  CHECK(new_in.str() == in.str());
}

TEST_CASE("ULTRA flex-mix (rus + eng) test ALL IN") {
  stringstream in, out, new_in;
  string s = R"LONG(You know my buddies saying
  Leck, leck, leck, leck, leck, leck, leck
  You know my girlies saying
  Leck, leck, leck, leck, leck, leck, leck
  Ah, and everybody saying
  Leck, leck, leck, leck, leck, leck, leck
  Yo, I know I gotta say it
  Leck, leck, leck, leck, leck, leck, leck (Ayy, Zoovier)

  [Куплет 1: Fetty Wap]
  Ayy, know you love a nigga style, little baby (Yeah)
  I got money, I can it throw for a while, little baby (Yeah)
  All blue hundrеds in the pile, little baby (Yеah)
  Love your pretty face, little smile, little baby
  Ayy, yeah, big bankroll out
  Call my nigga Hef, tell a nigga bring the hoes out
  Know I like a pretty yellow bone with her toes out (Ooh)
  Said it's war, super Blood, bring them poles out, yeah, baby
  Girl, you know I got you, baby (Yeah)
  Put you on the rocks too, baby (Yeah)
  Bring you to the block too, baby (Yeah)
  What you want? 'Cause I want you, yeah (Oow)
  Ah, ah, ah, ah, ah, ah, ah, in love with you (Yeah, baby, ooh)
  Drop, drop, drop, drop, drop, drop, drop, yeah (Yeah, baby)
  [Куплет 2: MORGENSHTERN]
  Сука, на мне часики Cartier (Дорого)
  Сука, на мне сука, как Cardi (B!)
  Pussy этой суки так горит (Хей)
  Хей, я плейбой, будто Carti
  Сука на мне, я зову её цепь (Цепь)
  Мы на войне, я зову её цель
  Эта сука на мне, она моя
  Значит никогда больше не будет твоей
  Папа comeback, выбиваю все двери (Хей)
  Папа — champagne, поливаю — всем fanny (Хей)
  Папа so dirty (Грязь), мне нужен FAIRY (Эй)
  Тут Imanbek, MORGEN и Fetty
  Собрались как-то раз америкос, казах, еврей (Хей)
  Ворвались на твой трап и забрали блядей (Хей)
  Собрались как-то раз америкос, казах, еврей
  (Хей, хей, хей, хей, хей, хей, хей)
  Сделали такой хит, шо весь мир охуел
  (Хей, хей, хей, хей, хей, хей, хей)
  — Месье Fetty Wap, будьте добры, скажите этим сукам

  [Припев: KDDK & Fetty Wap]
  You know my buddies saying
  Leck, leck, leck, leck, leck, leck, leck
  You know my girlies saying
  Leck, leck, leck, leck, leck, leck, leck
  Ah, and everybody saying
  Leck, leck, leck, leck, leck, leck, leck
  Yo, I know I gotta say it
  Leck, leck, leck, leck, leck, leck, leck (Ha))LONG";
  in << s;

  HuffArchiver archiver = HuffArchiver(in, out);
  archiver.zip();
  HuffArchiver unarchiver = HuffArchiver(out, new_in);
  unarchiver.unzip();
  CHECK(new_in.str() == in.str());
};

TEST_CASE("ALL IN encoding of an empty file") {
  stringstream in, out, new_in;
  HuffArchiver archiver = HuffArchiver(in, out);
  archiver.zip();
  HuffArchiver unarchiver = HuffArchiver(out, new_in);
  CompressionInfo result = unarchiver.unzip();
  CHECK(result.original == 0);
  CHECK(result.compressed == 0);
  CHECK(result.additional == 0);
  CHECK(new_in.str().length() == 0);
}

TEST_CASE("ALL IN encoding of bits stream") {
  stringstream in, out, new_in;
  vector<char> codes;
  for (int i = 0; i < CHAR_BIT; i++) {
    codes.push_back(char(i));
  }
  copy(codes.begin(), codes.end(), ostream_iterator<char>(in));
  CompressionInfo result1 = HuffArchiver(in, out).zip();
  CHECK(result1.original == CHAR_BIT);

  CompressionInfo result2 = HuffArchiver(out, new_in).unzip();
  CHECK(result2.compressed == CHAR_BIT);
}
