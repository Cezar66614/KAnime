#include "./jFunc.hpp"

void readJson(const char *fileName, json &out) { out.clear();
  std::FILE *input; input = std::fopen(fileName, "r");
  if (input) { std::string str; str.reserve(1048578); // alloc 1MB for string (avoid reallocation while reading)
    while (!std::feof(input)) str.push_back(std::getc(input)); str.pop_back();
    if (!str.empty()) out = json::parse(str);
  }
}
void writeJson(const char *fileName, json in) {
  std::FILE *output; output = std::fopen(fileName, "w");
  if (output) std::fprintf(output, "%s\n", in.dump(2).c_str());
  std::fclose(output);
}
