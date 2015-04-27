#ifndef NIHONGOPARSERD_FURIGANA_H
#define NIHONGOPARSERD_FURIGANA_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

class Furigana {
  private:
    static std::unordered_map<uint32_t, uint32_t> KataToHiraTable;
    static const char katakanas[][4];
    static const char hiraganas[][4];

  public:
    Furigana();
    std::string katakana_to_hiragana(std::string katakana);
    std::vector<std::pair<std::string, std::string> > tokenize(std::string kanji, std::string reading);
};

#endif
