#ifndef SINOPARSER_PARSER_H
#define SINOPARSER_PARSER_H

#include <mecab.h>
#include <string>
#include <vector>

#include "Furigana.h"

namespace MeCab {
    class Tagger;
}


class Parser {

    private:
        MeCab::Tagger* wakatiTagger;
        MeCab::Tagger* tagger;

    public:
        Furigana furigana;
        std::vector<std::pair<std::string, std::string> > tokenize(char const *str);
        Parser();
        ~Parser();

        // TODO move to private
        MeCab::Tagger* yomiTagger;
};


#endif

