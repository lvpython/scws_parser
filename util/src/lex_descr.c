#include "lex_descr.h"
#include <string.h>
int get_lextype(char* s_type)
{
  int i = 0;
  for(; i < 26; i++)
  {
    if (strcmp(l_descr[i].alias, s_type) == 0)
      return l_descr[i].lexid;
  }
  for(; i < 49; i++)
  {
    if (strcmp(l_descr[i].alias, s_type) == 0)
      return l_descr[13].lexid;
  }
  return l_descr[23].lexid;
}

static void fill_lex(int n, const char *alias, const char *description)
{
  l_descr[n].lexid = 97 + n;
  strcpy(l_descr[n].alias, alias);
  strcpy(l_descr[n].descr, description);
  descr[n].lexid = 97 + n;
  descr[n].alias = pstrdup(pool, alias);
  descr[n].descr = pstrdup(pool, description);
}

void init_lextype()
{
  /*
  * visit http://www.xunsearch.com/scws/docs.php#attr
  */
  /* pool used to management some dynamic memory */
  pool = pool_new();

  fill_lex(0, "a", "adjective");
  fill_lex(1, "b", "differentiation");
  fill_lex(2, "c", "conjunction");
  fill_lex(3, "d", "adverb");
  fill_lex(4, "e", "exclamation");
  fill_lex(5, "f", "position");
  fill_lex(6, "g", "root");
  fill_lex(7, "h", "head");
  fill_lex(8, "i", "idiom");
  fill_lex(9, "j", "abbreviation");
  fill_lex(10, "k", "head");
  fill_lex(11, "l", "tmp");
  fill_lex(12, "m", "numeral");
  fill_lex(13, "n", "noun");
  fill_lex(14, "o", "onomatopoeia");
  fill_lex(15, "p", "prepositional");
  fill_lex(16, "q", "quantity");
  fill_lex(17, "r", "pronoun");
  fill_lex(18, "s", "space");
  fill_lex(19, "t", "time");
  fill_lex(20, "u", "auxiliary");
  fill_lex(21, "v", "verb");
  fill_lex(22, "w", "punctuation");
  fill_lex(23, "x", "unknown");
  fill_lex(24, "y", "modal");
  fill_lex(25, "z", "status");
  fill_lex(26, "ag", "status (ag)");
  fill_lex(27, "ad", "status (ad)");
  fill_lex(28, "an", "status (an)");
  fill_lex(29, "dg", "status (dg)");
  fill_lex(30, "ng", "status (ng)");
  fill_lex(31, "nr", "status (nr)");
  fill_lex(32, "ns", "status (ns)");
  fill_lex(33, "nt", "status (nt)");
  fill_lex(34, "nz", "status (nz)");
  fill_lex(35, "ba", "status (ba)");
  fill_lex(36, "tg", "status (tg)");
  fill_lex(37, "vg", "status (vg)");
  fill_lex(38, "vd", "status (vd)");
  fill_lex(39, "vn", "status (vn)");
  fill_lex(40, "di", "status (di)");
  fill_lex(41, "as", "status (as)");
  fill_lex(42, "msp", "status (msp)");
  fill_lex(43, "dec", "status (dec)");
  fill_lex(44, "deg", "status (deg)");
  fill_lex(45, "etc", "status (etc)");
  fill_lex(46, "en", "english");
  fill_lex(47, "uj", "status (uj)");
  fill_lex(48, "un", "status (un)");
  fill_lex(49, "@", "status (@)");
}
