# contrib/zh_parser/Makefile

MODULE_big = scws_parser
OBJS = scws_parser.o
SCWS_HOME = /usr/local
EXTENSION = scws_parser
DATA = scws_parser--1.0.sql scws_parser--unpackaged--1.0.sql
DATA_TSEARCH = dict.utf8.xdb rules.utf8.ini chinese_utf8.stop

REGRESS = scws_parser

PG_CPPFLAGS = -I$(SCWS_HOME)/include/scws
SHLIB_LINK = -lscws -L$(SCWS_HOME)/lib -Wl,-rpath -Wl,$(SCWS_HOME)/lib

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

scws_OBJECTS = scws.o
scws_DEPENDENCIES = libscws.la
scws: $(scws_OBJECTS) $(scws_DEPENDENCIES)
	@rm -f scws
	$(LINK) $(scws_OBJECTS) $(scws_LDADD) $(LIBS)

%.o: src/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<
