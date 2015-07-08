# contrib/zh_parser/Makefile

MODULE_big = scws_parser
OBJS = scws_parser.o

EXTENSION = scws_parser
DATA = scws_parser--1.0.sql scws_parser--unpackaged--1.0.sql
DATA_TSEARCH = dict.utf8.xdb rules.utf8.ini

REGRESS = scws_parser

PG_CPPFLAGS = -I$(SCWS_HOME)/include/scws
SHLIB_LINK = -lscws -L$(SCWS_HOME)/lib -Wl,-rpath -Wl,$(SCWS_HOME)/lib

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
