#ifndef scws_parser_H
#define scws_parser_H

#ifndef pstrdup
#define pstrdup scws_pstrdup
#endif

#include "scws.h"

/* dict file extension */
#define TXT_EXT ".txt"
#define XDB_EXT ".xdb"
/* length of file extension */
#define EXT_LEN 4

/*
 * types
 */
typedef struct
{
  char *buffer;
  int   len;      /* length of the text in buffer */
  int   pos;      /* position of the parser */
  scws_t scws;
  scws_res_t head;
  scws_res_t curr;
} ParserState;

typedef struct
{
  int     lexid;
  char     *alias;
  char     *descr;
} LexDescr;

static void init_lextype(LexDescr descr[]);

#undef pstrdup

#endif
