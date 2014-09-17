#ifndef SINOPARSER_KANA_H
#define SINOPARSER_KANA_H

#include <unordered_map>
#include <iostream>

class Kana {
  private:
    static std::unordered_map<uint32_t, uint32_t> KataToHiraTable;
    static const char katakanas[][4];
    static const char hiraganas[][4];

  public:
    Kana();
    std::string katakana_to_hiragana(std::string katakana);
};

#endif
