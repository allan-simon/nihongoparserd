
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

/**
 * String reverse function copied from https://stackoverflow.com/questions/198199/how-do-you-reverse-a-string-in-place-in-c-or-c
 */

#define SWP(x,y) (x^=y, y^=x, x^=y)
static void strrev(char *p)
{
    char *q = p;
    while(q && *q) ++q; /* find eos */
    for(--q; p < q; ++p, --q) SWP(*p, *q);
}

/* Reverse a string in place, UTF-8 wise. */
void utf8_strrev(char *p)
{
  char *q = p;
  strrev(p); /* call base case */

  /* Ok, now fix bass-ackwards UTF chars. */
  while(q && *q) ++q; /* find eos */
  while(p < --q)
    switch( (*q & 0xF0) >> 4 ) {
    case 0xF: /* U+010000-U+10FFFF: four bytes. */
      SWP(*(q-0), *(q-3));
      SWP(*(q-1), *(q-2));
      q -= 3;
      break;
    case 0xE: /* U+000800-U+00FFFF: three bytes. */
      SWP(*(q-0), *(q-2));
      q -= 2;
      break;
    case 0xC: /* fall-through */
    case 0xD: /* U+000080-U+0007FF: two bytes. */
      SWP(*(q-0), *(q-1));
      q--;
      break;
    }
}
