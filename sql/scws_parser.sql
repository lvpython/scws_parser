DROP EXTENSION scws_parser;
CREATE EXTENSION scws_parser;

SELECT to_tsvector('scws_parser','革云士BNB净化器');
SELECT to_tsvector('scws_parser','马牌');
SELECT to_tsvector('scws_parser','保障房资金压力');
SELECT to_tsvector('scws_parser','舒语17寸轮毂');
SELECT ts_rank(to_tsvector('scws_parser',coalesce('BMW-8008-03M6')),(to_tsquery('scws_parser', ''' ' || 'bmw' || ' ''')));
