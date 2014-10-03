
#include "Kanji.h"

Kanji::Kanji() {
}

std::vector<std::pair<std::string, std::string> > Kanji::tokenize_kanjis(std::string kanjisString, std::string readingString) {
    std::vector<std::pair<std::string, std::string> > tokens;
    tokens.push_back(std::pair<std::string, std::string>(
        kanjisString,
        readingString
    ));
    return tokens;
}
