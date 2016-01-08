
#include <string.h>

#include "Parser.h"

/* Some labels or other strings used in the IPA dictionary. */
#define IPA_JOSHI "\u52a9\u8a5e" // 助詞
#define IPA_SETSUZOKUJOSHI "\u63a5\u7d9a\u52a9\u8a5e" // 接続助詞
#define IPA_BA "\u3070" // ば
#define IPA_TE "\u3066" // て
#define IPA_U "\u3046" // う
#define IPA_N "\u3093" // ん
#define IPA_FUHENKAKEI "\u4e0d\u5909\u5316\u578b" // 不変化型
#define IPA_JODOUSHI "\u52a9\u52d5\u8a5e" // 助動詞
#define IPA_TOKUSHU_NAI "\u7279\u6b8a\u30fb\u30ca\u30a4" // 特殊・ナイ
#define IPA_TOKUSHU_TA "\u7279\u6b8a\u30fb\u30bf" // 特殊・タ
#define IPA_TOKUSHU_TAI "\u7279\u6b8a\u30fb\u30bf\u30a4" // 特殊・タイ
#define IPA_TOKUSHU_MASU "\u7279\u6b8a\u30fb\u30de\u30b9" // 特殊・マス
#define IPA_TOKUSHU_NU "\u7279\u6b8a\u30fb\u30cc" // 特殊・ヌ
#define IPA_DOUSHI "\u52d5\u8a5e" // 動詞
#define IPA_SETSUBI "\u63a5\u5c3e" // 接尾
#define IPA_HIJIRITSU "\u975e\u81ea\u7acb" // 非自立

/* Number of comma-separated fields in the IPA dictionary. */
#define IPA_FEATURE_FIELDS 7

/* Number of the reading field in the IPA dictionary. */
#define IPA_FEATURE_READING_FIELD 8

/**
 * Parse the given comma-separated string into the provided array of strings.
 * "foo,bar,baz" → [ "foo", "bar", "baz" ]
 */
static void parse_feature(
    const char *feature,
    std::string (&parsed_feature)[IPA_FEATURE_FIELDS]
) {
    char *token, *infos = strdupa(feature);

    int i = 0;
    token = strtok(infos, ",");
    while (token != NULL && i < IPA_FEATURE_FIELDS) {
        parsed_feature[i].assign(token);
        i++;
        token = strtok(NULL, ",");
    }
}

/**
 * Check if the given parsed_feature matches against the given pattern.
 */
static bool pattern_match(
    std::string parsed_feature[IPA_FEATURE_FIELDS],
    std::string pattern[IPA_FEATURE_FIELDS]
) {
    for (int i = 0; i < IPA_FEATURE_FIELDS; i++) {
        if (!pattern[i].empty() && parsed_feature[i] != pattern[i]) {
            return false;
        }
    }
    return true;
}

/**
 * Check if the given feature (dictionary entry of a token)
 * should be merged with the preceding token.
 */
static bool is_mergeable_token(const char *feature) {
    /* A list of verb conjugation or suffixes patterns we'd like to keep
     * with the root. An empty string "" means wildcard. */
    static std::string ipa_merge_patterns[][IPA_FEATURE_FIELDS] = {
        /* 食べれ.ば */
        { IPA_JOSHI, IPA_SETSUZOKUJOSHI, "", "", "", "", IPA_BA },
        /* 食べ.て */
        { IPA_JOSHI, IPA_SETSUZOKUJOSHI, "", "", "", "", IPA_TE },
        /* 食べろ.う */
        { IPA_JODOUSHI, "", "", "", IPA_FUHENKAKEI, "", IPA_U },
        /* 食べませ.ん */
        { IPA_JODOUSHI, "", "", "", IPA_FUHENKAKEI, "", IPA_N },
        /* 食べ.ない */
        { IPA_JODOUSHI, "", "", "", IPA_TOKUSHU_NAI, "", "" },
        /* 食べ.たい */
        { IPA_JODOUSHI, "", "", "", IPA_TOKUSHU_TAI, "", "" },
        /* 食べ.ます */
        { IPA_JODOUSHI, "", "", "", IPA_TOKUSHU_MASU, "", "" },
        /* 変わら.ぬ */
        { IPA_JODOUSHI, "", "", "", IPA_TOKUSHU_NU, "", "" },
        /* 食べ.た, 食べ.たら */
        { IPA_JODOUSHI, "", "", "", IPA_TOKUSHU_TA, "", "" },
        /* Lots of verb suffixes like 食べ.られる, 食べ.させる,
         * 食べ.させて, さ.れる */
        { IPA_DOUSHI, IPA_SETSUBI, "", "", "", "", "" },
        /* Lots of -te verb suffixes like 食べ.ちゃう,
         * 食べて.いる/ある/くれる/もらう/あげる/やる/みる/しまう/
         *        いけない/なさい/おく,
         * and 食べなければ.ならない... */
        { IPA_DOUSHI, IPA_HIJIRITSU, "", "", "", "", "" }
    };

    std::string parsed_feature[IPA_FEATURE_FIELDS] = { "" };

    parse_feature(feature, parsed_feature);

    for (int i = 0; i < sizeof(ipa_merge_patterns)/sizeof(*ipa_merge_patterns); i++) {
        if (pattern_match(parsed_feature, ipa_merge_patterns[i])) {
            return true;
        }
    }
    return false;
}

/**
 * Return the wanted_field'th field of the feature of the given node.
 */
static std::string mecab_node_get_field(
    const MeCab::Node *node,
    int wanted_field
) {
    size_t field = 0;
    char *token = strdupa(node->feature);

    while (token != NULL) {
        char *next = NULL;

        if (*token == '"') {
            char *closing_quote = strchr(token + 1, '"');
            if (closing_quote && closing_quote[1] == ',') {
                next = closing_quote + 1;
                token++; // skip opening quote
                *closing_quote = '\0'; // remove closing quote
            }
        }
        if (!next)
            next = strchr(token, ',');
        if (next) {
            *next = '\0';
            next++;
        }

        if (field == wanted_field) {
            return std::string(token);
        }
        field++;
        token = next;
    }
    return "";
}

/**
 * Return the reading field of the given node (assuming IPA dictionary).
 */
static std::string mecab_node_get_reading(const MeCab::Node *node) {
    return mecab_node_get_field(node, IPA_FEATURE_READING_FIELD);
}

/**
 * Tokenize the given string into a list of (writing, reading) pairs,
 * for instance "私はここにいる。" should returns:
 * (
 *   ("私",   "わたし"),
 *   ("は",   "は"),
 *   ("ここ", "ここ"),
 *   ("に",   "に"),
 *   ("いる", "いる"),
 *   ("。",   "。")
 * )
 */
std::vector<std::pair<std::string, std::string> > Parser::tokenize(
    char const *str
) {
    std::vector<std::pair<std::string, std::string> > tokens;
    const MeCab::Node* node = this->tagger->parseToNode(str);
    for (; node; node = node->next) {
        if (node->stat != MECAB_BOS_NODE && node->stat != MECAB_EOS_NODE) {
            std::string token = std::string(node->surface, node->length);
            std::string reading = std::string(mecab_node_get_reading(node));

            if (tokens.size() > 0 && is_mergeable_token(node->feature)) {
                std::string prev_token = std::string(tokens.back().first);
                token.assign(prev_token + token);
                std::string prev_reading = std::string(tokens.back().second);
                reading.assign(prev_reading + reading);
                tokens.pop_back();
            }

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

