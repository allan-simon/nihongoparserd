
#include <cstring>
#include <algorithm>
#include <string.h>

#include "Furigana.h"
#include "Utf8.h"

std::unordered_map<uint32_t, uint32_t> Furigana::KataToHiraTable;

const char Furigana::katakanas[][4] = {
    "ァ","ア","ィ","イ","ゥ","ウ","ェ","エ","ォ","オ",
    "カ","ガ","キ","ギ","ク","グ","ケ","ゲ","コ","ゴ",
    "サ","ザ","シ","ジ","ス","ズ","セ","ゼ","ソ","ゾ",
    "タ","ダ","チ","ヂ","ッ","ツ","ヅ","テ","デ","ト",
    "ド","ナ","ニ","ヌ","ネ","ノ","ハ","バ","パ","ヒ",
    "ビ","ピ","フ","ブ","プ","ヘ","ベ","ペ","ホ","ボ",
    "ポ","マ","ミ","ム","メ","モ","ャ","ヤ","ュ","ユ",
    "ョ","ヨ","ラ","リ","ル","レ","ロ","ヮ","ワ","ヲ",
    "ン","ヴ","ヵ","ヶ"
};

const char Furigana::hiraganas[][4] = {
    "ぁ","あ","ぃ","い","ぅ","う","ぇ","え","ぉ","お",
    "か","が","き","ぎ","く","ぐ","け","げ","こ","ご",
    "さ","ざ","し","じ","す","ず","せ","ぜ","そ","ぞ",
    "た","だ","ち","ぢ","っ","つ","づ","て","で","と",
    "ど","な","に","ぬ","ね","の","は","ば","ぱ","ひ",
    "び","ぴ","ふ","ぶ","ぷ","へ","べ","ぺ","ほ","ぼ",
    "ぽ","ま","み","む","め","も","ゃ","や","ゅ","ゆ",
    "ょ","よ","ら","り","る","れ","ろ","ゎ","わ","を",
    "ん","ゔ","ゕ","ゖ"
};

Furigana::Furigana() {
    /* Initialize the katakana to hiragana hash table. */
    static size_t numKanas = sizeof(Furigana::katakanas)/sizeof(Furigana::katakanas[0]);
    for (int i = 0; i < numKanas; i++) {
        uint32_t katakana = *(uint32_t*)Furigana::katakanas[i];
        uint32_t hiragana = *(uint32_t*)Furigana::hiraganas[i];
        Furigana::KataToHiraTable.insert(std::make_pair(katakana, hiragana));
    }
}

/**
 * Return the given string with katakanas converted
 * to their hiragana equivalents.
 */
std::string Furigana::katakana_to_hiragana(std::string katakana)
{
    const char *str = katakana.c_str();
    char character[5] = { '\0' }; /* 4 UTF-8 bytes + '\0' = 5 */
    std::string hiraganas;

    while (utf8_getc(&str, character, sizeof(character)-1)) {
        auto res = Furigana::KataToHiraTable.find(*(uint32_t*)character);
        if (res != Furigana::KataToHiraTable.end()) {
            std::memcpy(character, &res->second, sizeof(character)-1);
        }

        hiraganas.append(character, std::strlen(character));
    }

    return hiraganas;
}

static inline void remove_spaces(std::string &str)
{
    str.erase(
        std::remove_if(
            str.begin(),
            str.end(),
            (int(*)(int))std::isspace),
        str.end()
    );
}

/**
 * Given two strings, check if they start with the same characters. If so,
 * return the length (in bytes) of equal part, or zero otherwise.
 * Return zero if the strings are equal.
 */
static size_t find_initial_equal_chars(char *kanjis_start, char *reading_start)
{
    const char *kanjis  = (const char*) kanjis_start;
    const char *reading = (const char*) reading_start;
    const char *kanjis_prev;
    char kanji_char[5] = { '\0' }; /* 4 UTF-8 bytes + '\0' = 5 */
    char  kana_char[5] = { '\0' }; /* 4 UTF-8 bytes + '\0' = 5 */

    if (strcmp(kanjis, reading) == 0) {
        return 0;
    }

    while (true) {
        kanjis_prev = kanjis;
        int more_kanjis  = utf8_getc(&kanjis, kanji_char, sizeof(kanji_char)-1);
        int more_reading = utf8_getc(&reading, kana_char, sizeof(kana_char)-1);
        if (!more_kanjis || !more_reading ||
            strncmp(kanji_char, kana_char, sizeof(kanji_char)) != 0) {
            break;
        }
    }
    return kanjis_prev - kanjis_start;
}

/**
 * Find how much we should trim the given reading and kanji strings.
 * Returns the length of head and tail to trim.
 */
static void find_trim_boundaries(
    std::string kanjis_std,
    std::string reading_std,
    size_t *start_len,
    size_t *end_len
) {
    char *kanjis  = strdup(kanjis_std.c_str());
    char *reading = strdup(reading_std.c_str());

    *start_len = find_initial_equal_chars(kanjis, reading);
    utf8_strrev(kanjis);
    utf8_strrev(reading);
    *end_len = find_initial_equal_chars(kanjis, reading);
    free(kanjis);
    free(reading);
}

/**
 * Split the given strings into one, two, or three parts, giving the
 * provided head and tail length.
 */
static std::vector<std::pair<std::string, std::string> > split_furigana(
    std::string kanjisString,
    std::string readingString,
    int start_len,
    int end_len
) {
    std::vector<std::pair<std::string, std::string> > tokens;
    tokens.push_back(std::pair<std::string, std::string>(
        kanjisString.substr(start_len, kanjisString.length() - end_len - start_len),
        readingString.substr(start_len, readingString.length() - end_len - start_len)
    ));
    if (start_len > 0) {
        tokens.insert(tokens.begin(), std::pair<std::string, std::string>(
            kanjisString.substr(0, start_len),
            readingString.substr(0, start_len)
        ));
    }
    if (end_len > 0) {
        tokens.push_back(std::pair<std::string, std::string>(
            kanjisString.substr(kanjisString.length() - end_len),
            readingString.substr(readingString.length() - end_len)
        ));
    }
    return tokens;
}

/**
 * Removes useless furiganas at the beginning and at the end.
 */
std::vector<std::pair<std::string, std::string> > Furigana::tokenize(
    std::string kanjisString,
    std::string readingString
) {
    size_t start_len, end_len;

    remove_spaces(readingString);
    readingString = this->katakana_to_hiragana(readingString);
    find_trim_boundaries(this->katakana_to_hiragana(kanjisString),
                         readingString,
                         &start_len,
                         &end_len);

    auto tokens = split_furigana(kanjisString,
                                 readingString,
                                 start_len,
                                 end_len);

    for (auto& text : tokens) {
        if (this->katakana_to_hiragana(text.first) == text.second) {
            text.second = "";
        }
    }

    return tokens;
}
