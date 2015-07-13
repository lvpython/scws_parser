LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

libscwsdir = $(top_srcdir)/../scws-1.2.2/libscws

AM_CPPFLAGS = -I$(top_srcdir) -I$(libscwsdir)

bin_PROGRAMS = $(top_builddir)/bin/scws

ACLOCAL_AMFLAGS = -I

__top_builddir__bin_scws_SOURCES = main.c lex_descr.c
__top_builddir__bin_scws_LDADD = $(libscwsdir)/libscws.la


