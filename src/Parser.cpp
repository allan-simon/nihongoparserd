
#include <string.h>

#include "Parser.h"

static std::string mecab_node_get_reading(const MeCab::Node *node) {
#define MECAB_FEATURE_READING_FIELD 8
    size_t field = 0;
    char *token, *infos = strdupa(node->feature);

    token = strtok(infos, ",");
    while (token != NULL) {
        field++;
        if (field == MECAB_FEATURE_READING_FIELD) {
            return std::string(token);
        }
        token = strtok(NULL, ",");
    }
    return "";
}

std::vector<std::pair<std::string, std::string> > Parser::tokenize(
    char const *str
) {
    std::vector<std::pair<std::string, std::string> > tokens;
    const MeCab::Node* node = this->tagger->parseToNode(str);
    for (; node; node = node->next) {
        if (node->stat != MECAB_BOS_NODE && node->stat != MECAB_EOS_NODE) {
            std::string token = std::string(node->surface, node->length);
            std::string reading = std::string(mecab_node_get_reading(node));

            tokens.push_back(std::pair<std::string, std::string>(
                token,
                reading
            ));
        }
    }
    return tokens;
}

Parser::Parser() {
    wakatiTagger = MeCab::createTagger("-Owakati");
    yomiTagger = MeCab::createTagger("-Oyomi");
    tagger = MeCab::createTagger("");
}

Parser::~Parser() {
    delete wakatiTagger;
    delete yomiTagger;
    delete tagger;
}

