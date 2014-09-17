
#include <cstring>
#include <cstdint>

#include "Kana.h"

std::unordered_map<uint32_t, uint32_t> Kana::KataToHiraTable;

const char Kana::katakanas[][4] = { 
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

const char Kana::hiraganas[][4] = { 
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

Kana::Kana() {
    /* Initialize the katakana to hiragana hash table. */
    for (int i = 0; i < sizeof(Kana::katakanas); i++) {
        uint32_t katakana = *(uint32_t*)Kana::katakanas[i];
        uint32_t hiragana = *(uint32_t*)Kana::hiraganas[i];
        Kana::KataToHiraTable.insert(std::make_pair(katakana, hiragana));
    }
}

/* Very naive UTF-8 parsing implementation. */
static int utf8_getc(const char **str, char *character, int characterLen) {
    /* mask values for bit pattern of first byte in multi-byte
     * UTF-8 sequences: 
     *   192 - 110xxxxx - for U+0080 to U+07FF 
     *   224 - 1110xxxx - for U+0800 to U+FFFF 
     *   240 - 11110xxx - for U+010000 to U+1FFFFF
     */
    const unsigned short mask[] = {255, 192, 224, 240};
    int i;

    for (i = 0; i < characterLen; i++) {
        character[i] = '\0';
    }

    character[0] = **str;
    for (
         i = 1;
         i < 4 && i < characterLen && **str != '\0'
              && (character[0] & mask[i]) == mask[i];
         i++
    ) {
        (*str)++;
        character[i] = **str;
    }
    (*str)++;

    return (character[0] != '\0');
}

std::string Kana::katakana_to_hiragana(std::string katakana) {
    const char *str = katakana.c_str();
    char character[5] = { '\0' }; /* 4 UTF-8 bytes + '\0' = 5 */
    std::string hiraganas;

    while (utf8_getc(&str, character, sizeof(character)-1)) {
        auto res = Kana::KataToHiraTable.find(*(uint32_t*)character);
        if (res != Kana::KataToHiraTable.end()) {
            std::memcpy(character, &res->second, sizeof(character)-1);
        }

        hiraganas.append(character, std::strlen(character));
    }

    return hiraganas;
}
