#ifndef LEX_DESCR_H
#define LEX_DESCR_H
// #ifndef pstrdup
//   #define pstrdup m_pstrdup
// #endif
#include "scws.h"
// #undef pstrdup


typedef struct
{
  int     lexid;
  char*    alias;
  char*    descr;
} LexDescr;

typedef struct
{
  int     lexid;
  char    alias[4];
  char    descr[20];
} TLexDescr;

static LexDescr   descr[50];
static TLexDescr  l_descr[50];
pool_t pool;


void init_lextype(void);
int get_lextype(char* s_type);
static void fill_lex(int n, const char *alias, const char *description);
#endif
