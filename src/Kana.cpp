
#include <cstring>
#include <cstdint>

#include "Utf8.h"
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
    static size_t numKanas = sizeof(Kana::katakanas)/sizeof(Kana::katakanas[0]);
    for (int i = 0; i < numKanas; i++) {
        uint32_t katakana = *(uint32_t*)Kana::katakanas[i];
        uint32_t hiragana = *(uint32_t*)Kana::hiraganas[i];
        Kana::KataToHiraTable.insert(std::make_pair(katakana, hiragana));
    }
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
