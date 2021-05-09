#include <iostream>
#include <fstream>
#include <array>
#include <cstring>
#include <string.h>

#include "huffman.h"

using namespace std;


struct my_arguments {
  string zip_mode;
  string input_file;
  string output_file;
};

my_arguments parse_terminal(int argc, char* argv[]) {
  my_arguments result;
  result.zip_mode = "";
  result.input_file = "";
  result.output_file = "";

  if(argc != 6) {
    throw runtime_error("Invalid number of arguments");
  }

  for(int i = 1; i < 6; i++) {
    if(!strcmp(argv[i], "-c")){
      if(result.zip_mode.length() == 0) {
        result.zip_mode = "zip";
      } else {
        throw runtime_error("Zip indicator must appear only once");
      }
    } else if (!strcmp(argv[i], "-u")) {
      if(result.zip_mode.length() == 0) {
        result.zip_mode = "unzip";
      } else {
        throw runtime_error("Zip indicator must appear only once");
      }
    } else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")) {
      if(result.input_file.length() == 0) {
        i++;
        result.input_file = argv[i];
      } else {
        throw runtime_error("Input file must appear only once");
      }
    } else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) {
      if(result.output_file.length() == 0) {
        i++;
        result.output_file = argv[i];
      } else {
        throw runtime_error("Output file must appear only once");
      }
    } else {
      throw runtime_error("Unknown argument: " + string(argv[i]));
    }
  }
  return result;
}

int main(int argc, char* argv[]) {
  try {
    my_arguments terminal = parse_terminal(argc, argv);
    string zip_mode = terminal.zip_mode;
    string input_file = terminal.input_file;
    string output_file = terminal.output_file;

    ifstream input(input_file, ios_base::binary);
    if(!input) {
      throw runtime_error("Invalid input file");
    }

    ofstream output(output_file, ios_base::binary);
    if(!output) {
      throw runtime_error("Invalid output file");
    }

    CompressionInfo result;
    HuffArchiver Archiver = HuffArchiver(input, output);

    if(!strcmp(reinterpret_cast <const char*> (&zip_mode), "zip")) {
      result = Archiver.zip();
    } else {
      result = Archiver.unzip();
    }

    cout << result.original << endl;
    cout << result.compressed << endl;
    cout << result.additional << endl;
  } catch(runtime_error& e) {
    cout << "Error happened:" << endl;
    cout << e.what() << endl;
  }
  return 0;
}
