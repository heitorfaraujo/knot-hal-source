AM_MAKEFLAGS = --no-print-directory

AM_CFLAGS = $(WARNING_CFLAGS) $(BUILD_CFLAGS) @GLIB_CFLAGS@
AM_LDFLAGS = $(BUILD_LDFLAGS)

libexec_PROGRAMS = rpi/rpiptd

rpi_rpiptd_SOURCES = rpi/main.c

rpi_rpiptd_LDADD = @GLIB_LIBS@
rpi_rpiptd_LDFLAGS = $(AM_LDFLAGS)
rpi_rpiptd_CFLAGS = $(AM_CFLAGS)

DISTCLEANFILES =

MAINTAINERCLEANFILES = Makefile.in \
	aclocal.m4 configure config.h.in config.sub config.guess \
	ltmain.sh depcomp compile missing install-sh

clean-local:
	$(RM) -r rpi/rpiptd