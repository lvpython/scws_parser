CREATE FUNCTION scwsprs_start(internal, int4)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION scwsprs_getlexeme(internal, internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION scwsprs_end(internal)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION scwsprs_lextype(internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE TEXT SEARCH PARSER scws_parser (
    START    = scwsprs_start,
    GETTOKEN = scwsprs_getlexeme,
    END      = scwsprs_end,
    HEADLINE = pg_catalog.prsd_headline,
    LEXTYPES = scwsprs_lextype
);

CREATE TEXT SEARCH DICTIONARY chinese_stem (
  TEMPLATE = snowball,
  Language = russian,
  StopWords = chinese_utf8
);

CREATE TEXT SEARCH CONFIGURATION scws_parser (PARSER = scws_parser);
ALTER TEXT SEARCH CONFIGURATION scws_parser ADD MAPPING FOR n,v,a,i,e,l,k,m,q WITH chinese_stem;
