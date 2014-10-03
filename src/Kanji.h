#ifndef NIHONGOPARSERD_KANJI_H
#define NIHONGOPARSERD_KANJI_H

#include <string>
#include <vector>
#include <unordered_map>

class Kanji {
  public:
    Kanji();
    std::vector<std::pair<std::string, std::string> > tokenize_kanjis(std::string kanji, std::string reading);
};

#endif
