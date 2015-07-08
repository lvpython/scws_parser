/*-------------------------------------------------------------------------
 *
 * scws_parser.c
 *	   text search parser for postgres
 *
 *-------------------------------------------------------------------------
 */
#include "scws_parser.h"

#include "postgres.h"
#include "miscadmin.h"
#include "fmgr.h"
#include "utils/guc.h"
#include "utils/builtins.h"


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

static bool lex_loaded = false;
/* config */
static bool dict_in_memory = true;
static char * extra_dicts = NULL;
static scws_t scws = NULL;
static bool punctuation_ignore = false;
static bool seg_with_duality = false;

static void init_scws(){
	char sharepath[MAXPGPATH];
	char dict_path[MAXPGPATH];
	char rule_path[MAXPGPATH];
	int load_dict_mem_mode = 0x0;

	List *elemlist;
	ListCell *l;
  if (scws != NULL) {
    return;
  }
	if (!(scws = scws_new())) {
		ereport(ERROR,
				(errcode(ERRCODE_INTERNAL_ERROR),
				 errmsg("Failed to init Chinese Parser Lib SCWS!\"%s\"",""
				       )));
	}

	DefineCustomBoolVariable(
		"scws_parser.dict_in_memory",
		"load dicts into memory",
		"load dicts into memory",
		&dict_in_memory,
		false,
		PGC_BACKEND,
		0,
		NULL,
		NULL,
		NULL
		);
	DefineCustomStringVariable(
		"scws_parser.extra_dicts",
		"extra dicts files to load",
		"extra dicts files to load",
		&extra_dicts,
		NULL,
		PGC_BACKEND,
		0,
		NULL,
		NULL,
		NULL
		);
	DefineCustomBoolVariable(
		"scws_parser.punctuation_ignore",
		"set if scws_parser ignores the puncuation",
		"set if scws_parser ignores the puncuation,except \\r and \\n",
		&punctuation_ignore,
		false,
		PGC_USERSET,
		0,
		NULL,
		NULL,
		NULL
		);

	DefineCustomBoolVariable(
		"scws_parser.seg_with_duality",
		"segment words with duality",
		"segment words with duality",
		&seg_with_duality,
		false,
		PGC_USERSET,
		0,
		NULL,
		NULL,
		NULL
		);

	get_share_path(my_exec_path, sharepath);
	snprintf(dict_path, MAXPGPATH, "%s/tsearch_data/%s.%s",
			sharepath, "dict.utf8", "xdb");
	scws_set_charset(scws, "utf8");

	if(dict_in_memory)
	    load_dict_mem_mode = SCWS_XDICT_MEM;

	/* ignore error,default dict is xdb */
	if( scws_set_dict(scws,dict_path,load_dict_mem_mode | SCWS_XDICT_XDB ) != 0){
	    ereport(NOTICE,
		    (errcode(ERRCODE_INTERNAL_ERROR),
		     errmsg("scws_parser set dict : \"%s\" failed!",dict_path
			 )));
	}

	if(extra_dicts != NULL){
	    if(!SplitIdentifierString(extra_dicts,',',&elemlist)){
		scws_free(scws);
		list_free(elemlist);
		scws = NULL;
		ereport(ERROR,
				(errcode(ERRCODE_INTERNAL_ERROR),
				 errmsg("scws_parser.extra_dicts syntax error! extra_dicts is \"%s\"",extra_dicts
				       )));
	    }

	    foreach(l,elemlist){
		int load_dict_mode = load_dict_mem_mode;
		char * ext = strrchr((char*)lfirst(l),'.');
		if(ext != NULL && strlen(ext) == EXT_LEN){
		    if(strncmp(ext,TXT_EXT,EXT_LEN) == 0){
			load_dict_mode |= SCWS_XDICT_TXT;
		    }
		    else if(strncmp(ext,XDB_EXT,EXT_LEN) == 0){
			load_dict_mode |= SCWS_XDICT_XDB;
		    }
		}

		if(((load_dict_mode & SCWS_XDICT_TXT) == 0) &&
			((load_dict_mode & SCWS_XDICT_XDB) == 0)){
			scws_free(scws);
			list_free(elemlist);
			scws = NULL;
			ereport(ERROR,
				(errcode(ERRCODE_INTERNAL_ERROR),
				 errmsg("scws_parser.extra_dicts setting error,the file name must end with .txt or .xdb! error file name is \"%s\"",(char*)lfirst(l)
				     )));

		}

		snprintf(dict_path, MAXPGPATH, "%s/tsearch_data/%s",
			sharepath, (char*)lfirst(l));
		/* ignore error*/
		if( scws_add_dict(scws,dict_path,load_dict_mode) != 0 ){
		    ereport(NOTICE,
			    (errcode(ERRCODE_INTERNAL_ERROR),
			     errmsg("scws_parser add dict : \"%s\" failed!",dict_path
				 )));
		}
	    }
	    list_free(elemlist);
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
	pst -> buffer = (char *) PG_GETARG_POINTER(0);
	pst -> len = PG_GETARG_INT32(1);
	pst -> pos = 0;
  pst -> scws = scws_fork(scws);
	scws_set_ignore(pst -> scws, (int)punctuation_ignore);
	scws_set_duality(pst -> scws,(int)seg_with_duality);

	scws_set_multi(pst -> scws, SCWS_MULTI_MASK);

	scws_send_text(pst -> scws, pst -> buffer, pst -> len);

	(pst -> head) = (pst -> curr) = scws_get_result(pst -> scws);

	PG_RETURN_POINTER(pst);
}

Datum
scwsprs_getlexeme(PG_FUNCTION_ARGS)
{
  ParserState *pst   = (ParserState *) PG_GETARG_POINTER(0);
  char        **t    = (char **) PG_GETARG_POINTER(1);
  int         *tlen  = (int *) PG_GETARG_POINTER(2);
  int         type   =  0;

	if((pst -> head) == NULL ) /* already done the work,or no sentence */
	{
		*tlen = 0;
		type = 0;
	}
	/* have results */
	else if(pst -> curr != NULL)
	{
		scws_res_t  curr = pst -> curr;

		/*
 		* check the first char to determine the lextype
 		* if out of [0,25],then set to 'x',mean unknown type
 		* so for Ag,Dg,Ng,Tg,Vg,the type will be unknown
 		* for full attr explanation,visit http://www.xunsearch.com/scws/docs.php#attr
		*/
		type = (int)(curr -> attr)[0];
		if(type > (int)'x' || type < (int)'a')
		    type = (int)'x';
		*tlen = curr -> len;
		*t = pst -> buffer + curr -> off;

		pst -> curr = curr -> next;

		/* fetch the next sentence */
		if(pst -> curr == NULL ){
			scws_free_result(pst -> head);
			(pst -> head) =	(pst -> curr) = scws_get_result(pst -> scws);
		}
	}

	PG_RETURN_INT32(type);
}

Datum
scwsprs_end(PG_FUNCTION_ARGS)
{
  ParserState *pst = (ParserState *) PG_GETARG_POINTER(0);
  scws_free(pst -> scws);
  pfree(pst);
	PG_RETURN_VOID();
}

Datum
scwsprs_lextype(PG_FUNCTION_ARGS)
{
	static LexDescr   descr[27];
	if(lex_loaded == 0){
	    init_lextype(descr);
	    lex_loaded = 1;
	}

	PG_RETURN_POINTER(descr);
}

static void init_lextype(LexDescr descr[]){
	/*
	* there are 26 types in this parser,alias from a to z
	* for full attr explanation,visit http://www.xunsearch.com/scws/docs.php#attr
	*/
	descr[0].lexid = 97;
	descr[0].alias = pstrdup("a");
	descr[0].descr = pstrdup("adjective");
	descr[1].lexid = 98;
	descr[1].alias = pstrdup("b");
	descr[1].descr = pstrdup("differentiation (qu bie)");
	descr[2].lexid = 99;
	descr[2].alias = pstrdup("c");
	descr[2].descr = pstrdup("conjunction");
	descr[3].lexid = 100;
	descr[3].alias = pstrdup("d");
	descr[3].descr = pstrdup("adverb");
	descr[4].lexid = 101;
	descr[4].alias = pstrdup("e");
	descr[4].descr = pstrdup("exclamation");
	descr[5].lexid = 102;
	descr[5].alias = pstrdup("f");
	descr[5].descr = pstrdup("position (fang wei)");
	descr[6].lexid = 103;
	descr[6].alias = pstrdup("g");
	descr[6].descr = pstrdup("root (ci gen)");
	descr[7].lexid = 104;
	descr[7].alias = pstrdup("h");
	descr[7].descr = pstrdup("head");
	descr[8].lexid = 105;
	descr[8].alias = pstrdup("i");
	descr[8].descr = pstrdup("idiom");
	descr[9].lexid = 106;
	descr[9].alias = pstrdup("j");
	descr[9].descr = pstrdup("abbreviation (jian lue)");
	descr[10].lexid = 107;
	descr[10].alias = pstrdup("k");
	descr[10].descr = pstrdup("head");
	descr[11].lexid = 108;
	descr[11].alias = pstrdup("l");
	descr[11].descr = pstrdup("tmp (lin shi)");
	descr[12].lexid = 109;
	descr[12].alias = pstrdup("m");
	descr[12].descr = pstrdup("numeral");
	descr[13].lexid = 110;
	descr[13].alias = pstrdup("n");
	descr[13].descr = pstrdup("noun");
	descr[14].lexid = 111;
	descr[14].alias = pstrdup("o");
	descr[14].descr = pstrdup("onomatopoeia");
	descr[15].lexid = 112;
	descr[15].alias = pstrdup("p");
	descr[15].descr = pstrdup("prepositional");
	descr[16].lexid = 113;
	descr[16].alias = pstrdup("q");
	descr[16].descr = pstrdup("quantity");
	descr[17].lexid = 114;
	descr[17].alias = pstrdup("r");
	descr[17].descr = pstrdup("pronoun");
	descr[18].lexid = 115;
	descr[18].alias = pstrdup("s");
	descr[18].descr = pstrdup("space");
	descr[19].lexid = 116;
	descr[19].alias = pstrdup("t");
	descr[19].descr = pstrdup("time");
	descr[20].lexid = 117;
	descr[20].alias = pstrdup("u");
	descr[20].descr = pstrdup("auxiliary");
	descr[21].lexid = 118;
	descr[21].alias = pstrdup("v");
	descr[21].descr = pstrdup("verb");
	descr[22].lexid = 119;
	descr[22].alias = pstrdup("w");
	descr[22].descr = pstrdup("punctuation (qi ta biao dian)");
	descr[23].lexid = 120;
	descr[23].alias = pstrdup("x");
	descr[23].descr = pstrdup("unknown");
	descr[24].lexid = 121;
	descr[24].alias = pstrdup("y");
	descr[24].descr = pstrdup("modal (yu qi)");
	descr[25].lexid = 122;
	descr[25].alias = pstrdup("z");
	descr[25].descr = pstrdup("status (zhuang tai)");
	descr[26].lexid = 0;
}
//TODO :headline function
