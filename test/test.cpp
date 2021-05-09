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

TEST_CASE("test #2 of the table function")  {
  stringstream in;
  in << "Остановиста, бэйби";

  array<size_t, 256> table = exctract_counts(in);
  // CHECK(table[to_code('б')] == 0);
  // CHECK(table[to_code('й')] == 0);
  CHECK(table[','] == 1);
  CHECK(table['.'] == 0);
}

TEST_CASE("test #3 of the table function") {
  stringstream in;
  in << "";

  array<size_t, 256> table = exctract_counts(in);
  // CHECK(table[to_code('л')] == 0);
  // CHECK(table[to_code('ф')] == 0);
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
};

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

  char* slovo = new char[5];
  reader.read(slovo, 5);

  CHECK(!strcmp(slovo, "salam"));
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

TEST_CASE("Big English test for ALL IN encoding") {
  stringstream in, out, new_in;
  string s = R"LONG(I gaze in sorrow upon our generation!
  Its future is either dark or empty,
  And under a burden of knowledge and doubt,
  It will grow old, having accomplished nothing.
  We are aware, straight from our cribs,
  Of our forefather's mistakes and late maturity;
  And thus our lives are tedious, like a path with no goal,
  Like a feast at a foreign celebration.
  For shame! We are indifferent to both good and evil,
  And fade without a battle at the start of our careers;
  We are dishonorable cowards in the face of danger,
  And despicable slaves before authority.
  We are like a non-ripe fruit,
  (That orphaned stranger hanging amongst flowers)
  Which brings no joy to our eyes or mouths,
  And falls when the flowers bloom!
  Fruitless science has dried our minds,
  And we jealously guard from friends and family
  Our best hopes and noble voice,
  For our passions have been ridiculed.
  We have barely touched the cup of pleasure,
  And yet we managed to waste our youthful strength;
  From every joy, afraid of being full,
  We drank once, and that was enough.
  The dreams of poetry and works of art
  Do not instill in us delightful awe;
  We greedily cherish what feeling remains in our heart,
  For it is a useless treasure, buried by miserliness.
  By chance we love and hate,
  Giving nothing to either anger or love,
  And a strange chill remains in our souls
  Even while our blood boils.
  We are bored by the opulent amusements of our forefathers,
  By their honest and lighthearted debauchery;
  And even to our graves we hurry without joy or glory,
  But look back with derision.
  As part of a sombre and soon forgotten crowd
  We will quietly pass over the world, leaving nary a trace,
  Nor a fruitful thought for eternity to ponder over,
  Nor the genius of some starter work.
  And a descendant, with the strictness of both judge and jury,
  Will derisively speak of our remains.
  He will know only the bitter scorn of a son
  Whose father wasted a life in vain.)LONG";
  in << s;

  HuffArchiver archiver = HuffArchiver(in, out);
  archiver.zip();
  HuffArchiver unarchiver = HuffArchiver(out, new_in);
  unarchiver.unzip();
  CHECK(new_in.str() == in.str());
};

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
};

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

TEST_CASE("Empty file") {
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

TEST_CASE("Wiki test: 15 7 6 6 5") {
  string a(15, 'a');
  string b(7, 'b');
  string c(6, 'c');
  string d(6, 'd');
  stringstream in, out, decoded_out;
  in << a << b << c << d << '\0';
  HuffArchiver archiver = HuffArchiver(in, out);
  CompressionInfo result = archiver.zip();
  CHECK(result.compressed == 87 / CHAR_BIT);
}

TEST_CASE("Fibonacci freqs stress test" * doctest::timeout(5.0)) {
  vector<int> fib = {1,     1,     2,     3,    5,    8,    13,
                          21,    34,    55,    89,   144,  233,  377,
                          610,   987,   1597,  2584, 4181, 6765, 10946,
                          17711, 28657, 46368, 75025};
  stringstream in, out, new_in;

  int start_code = 5;
  for (auto const f : fib) {
    for (int i = 0; i < f; i++)
      in << char(start_code);
    start_code++;
  }
  HuffArchiver archiver = HuffArchiver(in, out);
  archiver.zip();
  HuffArchiver unarchiver = HuffArchiver(out, new_in);
  unarchiver.unzip();
  CHECK(new_in.str().length() == 196418 - 1); // F(n + 2) - 1
}

TEST_CASE("Stress testing 5mb file" * doctest::timeout(5.0)) {
  const int chars = 5000000;
  stringstream in, out, new_in;
  vector<char> codes;
  for (int i = 0; i < chars; i++) {
    int index = rand() % 128;
    codes.push_back(char(index));
  }
  copy(codes.begin(), codes.end(), ostream_iterator<char>(in));
  HuffArchiver archiver = HuffArchiver(in, out);
  archiver.zip();
  HuffArchiver unarchiver = HuffArchiver(out, new_in);
  unarchiver.unzip();
  CHECK(in.str() == new_in.str());
}

TEST_CASE("Encoding creation for CHAR_BIT bits stream") {
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

TEST_CASE("Encoding creation for CHAR_BIT + 1 bits stream") {
  stringstream in, out, new_in;
  vector<char> codes;
  for (int i = 0; i < CHAR_BIT + 1; i++) {
    codes.push_back(char(i));
  }
  copy(codes.begin(), codes.end(), ostream_iterator<char>(in));

  CompressionInfo result1 = HuffArchiver(in, out).zip();
  CHECK(result1.original == CHAR_BIT + 1);

  CompressionInfo result2 = HuffArchiver(out, new_in).unzip();
  CHECK(result2.compressed == CHAR_BIT + 1);
}
