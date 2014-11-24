#define TRUE 1
#define FALSE 0

#include "englishstemmer.h"

EnglishStemmer::EnglishStemmer() {
    z = (struct stemmer*)malloc(sizeof(struct stemmer));
}

EnglishStemmer::~EnglishStemmer() {
    free(z);
}


/* cons(z, i) is TRUE <=> b[i] is a consonant. ('b' means 'z->b', but here
   and below we drop 'z->' in comments.
*/

int EnglishStemmer::cons(int i)
{  switch (z->b[i])
   {  case 'a': case 'e': case 'i': case 'o': case 'u': return FALSE;
      case 'y': return (i == 0) ? TRUE : !cons(i - 1);
      default: return TRUE;
   }
}

/* m(z) measures the number of consonant sequences between 0 and j. if c is
   a consonant sequence and v a vowel sequence, and <..> indicates arbitrary
   presence,

      <c><v>       gives 0
      <c>vc<v>     gives 1
      <c>vcvc<v>   gives 2
      <c>vcvcvc<v> gives 3
      ....
*/

int EnglishStemmer::m()
{  int n = 0;
   int i = 0;
   int j = z->j;
   while(TRUE)
   {  if (i > j) return n;
      if (! cons(i)) break; i++;
   }
   i++;
   while(TRUE)
   {  while(TRUE)
      {  if (i > j) return n;
            if (cons(i)) break;
            i++;
      }
      i++;
      n++;
      while(TRUE)
      {  if (i > j) return n;
         if (! cons(i)) break;
         i++;
      }
      i++;
   }
}

/* vowelinstem(z) is TRUE <=> 0,...j contains a vowel */

int EnglishStemmer::vowelinstem()
{
   int j = z->j;
   int i; for (i = 0; i <= j; i++) if (! cons(i)) return TRUE;
   return FALSE;
}

/* doublec(z, j) is TRUE <=> j,(j-1) contain a double consonant. */

int EnglishStemmer::doublec(int j)
{
   char * b = z->b;
   if (j < 1) return FALSE;
   if (b[j] != b[j - 1]) return FALSE;
   return cons(j);
}

/* cvc(z, i) is TRUE <=> i-2,i-1,i has the form consonant - vowel - consonant
   and also if the second c is not w,x or y. this is used when trying to
   restore an e at the end of a short word. e.g.

      cav(e), lov(e), hop(e), crim(e), but
      snow, box, tray.

*/

int EnglishStemmer::cvc(int i)
{  if (i < 2 || !cons(i) || cons(i - 1) || !cons(i - 2)) return FALSE;
   {  int ch = z->b[i];
      if (ch  == 'w' || ch == 'x' || ch == 'y') return FALSE;
   }
   return TRUE;
}

/* ends(z, s) is TRUE <=> 0,...k ends with the string s. */

int EnglishStemmer::ends(char * s)
{  int length = s[0];
   char * b = z->b;
   int k = z->k;
   if (s[length] != b[k]) return FALSE; /* tiny speed-up */
   if (length > k + 1) return FALSE;
   if (memcmp(b + k - length + 1, s + 1, length) != 0) return FALSE;
   z->j = k-length;
   return TRUE;
}

/* setto(z, s) sets (j+1),...k to the characters in the string s, readjusting
   k. */

void EnglishStemmer::setto(char * s)
{  int length = s[0];
   int j = z->j;
   memmove(z->b + j + 1, s + 1, length);
   z->k = j+length;
}

/* r(z, s) is used further down. */

void EnglishStemmer::r(char * s) { if (m() > 0) setto(s); }

/* step1ab(z) gets rid of plurals and -ed or -ing. e.g.

       caresses  ->  caress
       ponies    ->  poni
       ties      ->  ti
       caress    ->  caress
       cats      ->  cat

       feed      ->  feed
       agreed    ->  agree
       disabled  ->  disable

       matting   ->  mat
       mating    ->  mate
       meeting   ->  meet
       milling   ->  mill
       messing   ->  mess

       meetings  ->  meet

*/

void EnglishStemmer::step1ab()
{
   char * b = z->b;
   if (b[z->k] == 's')
   {  if (ends("\04" "sses")) z->k -= 2; else
      if (ends("\03" "ies")) setto("\01" "i"); else
      if (b[z->k - 1] != 's') z->k--;
   }
   if (ends("\03" "eed")) { if (m() > 0) z->k--; } else
   if ((ends("\02" "ed") || ends("\03" "ing")) && vowelinstem())
   {  z->k = z->j;
      if (ends("\02" "at")) setto("\03" "ate"); else
      if (ends("\02" "bl")) setto("\03" "ble"); else
      if (ends("\02" "iz")) setto("\03" "ize"); else
      if (doublec(z->k))
      {  z->k--;
         {  int ch = b[z->k];
            if (ch == 'l' || ch == 's' || ch == 'z') z->k++;
         }
      }
      else if (m() == 1 && cvc(z->k)) setto("\01" "e");
   }
}

/* step1c(z) turns terminal y to i when there is another vowel in the stem. */

void EnglishStemmer::step1c()
{
   if (ends("\01" "y") && vowelinstem()) z->b[z->k] = 'i';
}


/* step2(z) maps double suffices to single ones. so -ization ( = -ize plus
   -ation) maps to -ize etc. note that the string before the suffix must give
   m(z) > 0. */

void EnglishStemmer::step2() { switch (z->b[z->k-1])
{
   case 'a': if (ends("\07" "ational")) { r("\03" "ate"); break; }
             if (ends("\06" "tional")) { r("\04" "tion"); break; }
             break;
   case 'c': if (ends("\04" "enci")) { r("\04" "ence"); break; }
             if (ends("\04" "anci")) { r("\04" "ance"); break; }
             break;
   case 'e': if (ends("\04" "izer")) { r("\03" "ize"); break; }
             break;
   case 'l': if (ends("\03" "bli")) { r("\03" "ble"); break; } /*-DEPARTURE-*/

 /* To match the published algorithm, replace this line with
    case 'l': if (ends(z, "\04" "abli")) { r(z, "\04" "able"); break; } */

             if (ends("\04" "alli")) { r("\02" "al"); break; }
             if (ends("\05" "entli")) { r("\03" "ent"); break; }
             if (ends("\03" "eli")) { r("\01" "e"); break; }
             if (ends("\05" "ousli")) { r("\03" "ous"); break; }
             break;
   case 'o': if (ends("\07" "ization")) { r("\03" "ize"); break; }
             if (ends("\05" "ation")) { r("\03" "ate"); break; }
             if (ends("\04" "ator")) { r("\03" "ate"); break; }
             break;
   case 's': if (ends("\05" "alism")) { r("\02" "al"); break; }
             if (ends("\07" "iveness")) { r("\03" "ive"); break; }
             if (ends("\07" "fulness")) { r("\03" "ful"); break; }
             if (ends("\07" "ousness")) { r("\03" "ous"); break; }
             break;
   case 't': if (ends("\05" "aliti")) { r("\02" "al"); break; }
             if (ends("\05" "iviti")) { r("\03" "ive"); break; }
             if (ends("\06" "biliti")) { r("\03" "ble"); break; }
             break;
   case 'g': if (ends("\04" "logi")) { r("\03" "log"); break; } /*-DEPARTURE-*/

 /* To match the published algorithm, delete this line */

} }

/* step3(z) deals with -ic-, -full, -ness etc. similar strategy to step2. */

void EnglishStemmer::step3() { switch (z->b[z->k])
{
   case 'e': if (ends("\05" "icate")) { r("\02" "ic"); break; }
             if (ends("\05" "ative")) { r("\00" ""); break; }
             if (ends("\05" "alize")) { r("\02" "al"); break; }
             break;
   case 'i': if (ends("\05" "iciti")) { r("\02" "ic"); break; }
             break;
   case 'l': if (ends("\04" "ical")) { r("\02" "ic"); break; }
             if (ends("\03" "ful")) { r("\00" ""); break; }
             break;
   case 's': if (ends("\04" "ness")) { r("\00" ""); break; }
             break;
} }

/* step4(z) takes off -ant, -ence etc., in context <c>vcvc<v>. */

void EnglishStemmer::step4()
{  switch (z->b[z->k-1])
   {  case 'a': if (ends("\02" "al")) break; return;
      case 'c': if (ends("\04" "ance")) break;
                if (ends("\04" "ence")) break; return;
      case 'e': if (ends("\02" "er")) break; return;
      case 'i': if (ends("\02" "ic")) break; return;
      case 'l': if (ends("\04" "able")) break;
                if (ends("\04" "ible")) break; return;
      case 'n': if (ends("\03" "ant")) break;
                if (ends("\05" "ement")) break;
                if (ends("\04" "ment")) break;
                if (ends("\03" "ent")) break; return;
      case 'o': if (ends("\03" "ion") && (z->b[z->j] == 's' || z->b[z->j] == 't')) break;
                if (ends("\02" "ou")) break; return;
                /* takes care of -ous */
      case 's': if (ends("\03" "ism")) break; return;
      case 't': if (ends("\03" "ate")) break;
                if (ends("\03" "iti")) break; return;
      case 'u': if (ends("\03" "ous")) break; return;
      case 'v': if (ends("\03" "ive")) break; return;
      case 'z': if (ends("\03" "ize")) break; return;
      default: return;
   }
   if (m() > 1) z->k = z->j;
}

/* step5(z) removes a final -e if m(z) > 1, and changes -ll to -l if
   m(z) > 1. */

void EnglishStemmer::step5()
{
   char * b = z->b;
   z->j = z->k;
   if (b[z->k] == 'e')
   {  int a = m();
      if (a > 1 || a == 1 && !cvc(z->k - 1)) z->k--;
   }
   if (b[z->k] == 'l' && doublec(z->k) && m() > 1) z->k--;
}

/* In stem(z, b, k), b is a char pointer, and the string to be stemmed is
   from b[0] to b[k] inclusive.  Possibly b[k+1] == '\0', but it is not
   important. The stemmer adjusts the characters b[0] ... b[k] and returns
   the new end-point of the string, k'. Stemming never increases word
   length, so 0 <= k' <= k.
*/

int EnglishStemmer::do_stem(char * b, int k)
{
   if (k <= 1) return k; /*-DEPARTURE-*/
   z->b = b; z->k = k; /* copy the parameters into z */

   /* With this line, strings of length 1 or 2 don't go through the
      stemming process, although no mention is made of this in the
      published algorithm. Remove the line to match the published
      algorithm. */

   step1ab(); step1c(); step2(); step3(); step4(); step5();
   return z->k;
}

/*--------------------stemmer definition ends here------------------------*/
