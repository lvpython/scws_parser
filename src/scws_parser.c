/*-------------------------------------------------------------------------
 *
 * scws_parser.c
 *     text search parser for postgres
 *
 *-------------------------------------------------------------------------
 */
#include "scws_parser.h"


#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif
/*
 * prototypes
 */
PG_FUNCTION_INFO_V1(scwsprs_start);
Datum scwsprs_start(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(scwsprs_getlexeme);
Datum scwsprs_getlexeme(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(scwsprs_end);
Datum scwsprs_end(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(scwsprs_lextype);
Datum scwsprs_lextype(PG_FUNCTION_ARGS);

/* config */
static bool dict_in_memory = true;
static scws_t scws = NULL;
static bool punctuation_ignore = false;
static bool seg_with_duality = false;

static LexDescr descr[50];

static void define_custom_variable()
{
  DefineCustomBoolVariable(
    "scws_parser.dict_in_memory",
    "load dicts into memory",
    "load dicts into memory",
    &dict_in_memory, false, PGC_BACKEND,
    0, NULL, NULL, NULL);
  DefineCustomBoolVariable(
    "scws_parser.punctuation_ignore",
    "set if scws_parser ignores the puncuation",
    "set if scws_parser ignores the puncuation,except \\r and \\n",
    &punctuation_ignore, false, PGC_USERSET,
    0, NULL, NULL, NULL);

  DefineCustomBoolVariable(
    "scws_parser.seg_with_duality",
    "segment words with duality",
    "segment words with duality",
    &seg_with_duality, false, PGC_USERSET,
    0, NULL, NULL, NULL);
}
static void init_scws(){
  char sharepath[MAXPGPATH];
  char dict_path[MAXPGPATH];
  char rule_path[MAXPGPATH];
  int load_dict_mem_mode = 0x0;

  if (scws != NULL) {
    return;
  }
  if (!(scws = scws_new())) {
    ereport(ERROR,(errcode(ERRCODE_INTERNAL_ERROR),
                   errmsg("Failed to init Chinese Parser Lib SCWS!\"%s\"",""
                   )));
  }
  define_custom_variable();
  get_share_path(my_exec_path, sharepath);
  snprintf(dict_path, MAXPGPATH, "%s/tsearch_data/%s.%s",
      sharepath, "dict.utf8", "xdb");
  scws_set_charset(scws, "utf8");

  if(dict_in_memory)
      load_dict_mem_mode = SCWS_XDICT_MEM;

  /* ignore error,default dict is xdb */
  if( scws_set_dict(scws,dict_path,load_dict_mem_mode | SCWS_XDICT_XDB ) != 0){
      ereport(NOTICE, (errcode(ERRCODE_INTERNAL_ERROR),
                       errmsg("scws_parser set dict : \"%s\" failed!",dict_path
                       )));
  }

  snprintf(rule_path, MAXPGPATH, "%s/tsearch_data/%s.%s",
      sharepath, "rules.utf8", "ini");
  scws_set_rule(scws ,rule_path);
}

/*
 * functions
 */
Datum
scwsprs_start(PG_FUNCTION_ARGS)
{
  init_scws();

  ParserState *pst = (ParserState *) palloc(sizeof(ParserState));
  pst->buffer = (char *) PG_GETARG_POINTER(0);
  pst->len = PG_GETARG_INT32(1);
  pst->pos = 0;
  pst->scws = scws_fork(scws);

  scws_set_ignore(pst->scws, (int)punctuation_ignore);
  scws_set_duality(pst->scws,(int)seg_with_duality);
  scws_set_multi(pst->scws, SCWS_MULTI_MASK);
  scws_send_text(pst->scws, pst->buffer, pst->len);

  (pst->head) = (pst->curr) = scws_get_result(pst->scws);

  PG_RETURN_POINTER(pst);
}

Datum
scwsprs_getlexeme(PG_FUNCTION_ARGS)
{
  ParserState *pst   = (ParserState *) PG_GETARG_POINTER(0);
  char        **t    = (char **) PG_GETARG_POINTER(1);
  int         *tlen  = (int *) PG_GETARG_POINTER(2);
  int         type   =  0;
  char        s_type[4];

  if((pst->head) == NULL ) /* already done the work,or no sentence */
  {
    *tlen = 0;
    type = 0;
  }
  /* have results */
  else if(pst->curr != NULL)
  {
    scws_res_t  curr = pst->curr;

    sprintf(s_type,"%.3s", curr->attr);
    if (strlen(s_type)==1) {
      type = curr->attr[0];
    }
    else {
      type = 110;
    }
    *tlen = curr->len;
    *t = pst->buffer + curr->off;

    pst->curr = curr->next;

    /* fetch the next sentence */
    if(pst->curr == NULL ){
      scws_free_result(pst->head);
      (pst->head) = (pst->curr) = scws_get_result(pst->scws);
    }
  }

  PG_RETURN_INT32(type);
}

Datum
scwsprs_end(PG_FUNCTION_ARGS)
{
  ParserState *pst = (ParserState *) PG_GETARG_POINTER(0);
  scws_free(pst->scws);
  pfree(pst);
  PG_RETURN_VOID();
}

Datum
scwsprs_lextype(PG_FUNCTION_ARGS)
{
  init_lextype();

  PG_RETURN_POINTER(descr);
}

static void fill_lex(int n, const char *alias, const char *description)
{
  descr[n].lexid = 97 + n;
  descr[n].alias = pstrdup(alias);
  descr[n].descr = pstrdup(description);
}

static void init_lextype(){
  /*
  * visit http://www.xunsearch.com/scws/docs.php#attr
  */

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
