
/* Very naive UTF-8 parsing implementation. */
int utf8_getc(const char **str, char *character, int characterLen) {
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
