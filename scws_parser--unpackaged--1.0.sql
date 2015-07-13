-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION scws_parser" to load this file. \quit

ALTER EXTENSION scws_parser ADD function scwsprs_start(internal,integer);
ALTER EXTENSION scws_parser ADD function scwsprs_getlexeme(internal,internal,internal);
ALTER EXTENSION scws_parser ADD function scwsprs_end(internal);
ALTER EXTENSION scws_parser ADD function scwsprs_lextype(internal);
ALTER EXTENSION scws_parser ADD text search parser scws_parser;
ALTER EXTENSION scws_parser ADD TEXT SEARCH CONFIGURATION scws_parser;
ALTER EXTENSION scws_parser ADD TEXT SEARCH DICTIONARY chinese_stem;
ALTER EXTENSION scws_parser ALTER TEXT SEARCH CONFIGURATION scws_parser;
-- add token mapping

