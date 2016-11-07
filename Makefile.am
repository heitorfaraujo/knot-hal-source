AM_MAKEFLAGS = --no-print-directory
SUBDIRS = src/spi src/nrf24l01 src/hal/storage src/hal/time src/hal/log \
			src/hal/comm src/drivers

AM_CFLAGS = $(WARNING_CFLAGS) $(BUILD_CFLAGS)
AM_LDFLAGS = $(BUILD_LDFLAGS)

lib_ARDUINO = include/comm.h include/log.h include/nrf24.h include/storage.h \
				include/time.h src/avr_errno.h src/avr_unistd.h

bin_PROGRAMS = proxy/spiproxyd src/nrfd/nrfd src/phyemud/phyemud \
							examples/uthingd tools/phyd_test

proxy_spiproxyd_SOURCES = proxy/main.c
proxy_spiproxyd_LDADD = libs/libspi.a libs/libnrf24l01.a @GLIB_LIBS@
proxy_spiproxyd_LDFLAGS = $(AM_LDFLAGS)
proxy_spiproxyd_CFLAGS = $(AM_CFLAGS) @GLIB_CFLAGS@

src_nrfd_nrfd_SOURCES = src/nrfd/main.c \
				src/nrfd/manager.h src/nrfd/manager.c
src_nrfd_nrfd_LDADD = libs/libhallog.a \
				libs/libhalcomm.a \
				libs/libhaltime.a \
				libs/libhalstorage.a \
				libs/libnrf24l01.a \
				libs/libspi.a \
				@GLIB_LIBS@
src_nrfd_nrfd_LDFLAGS = $(AM_LDFLAGS)
src_nrfd_nrfd_CFLAGS = $(AM_CFLAGS) @GLIB_CFLAGS@ \
				-I$(top_srcdir)/src \
				-I$(top_srcdir)/src/hal/comm

src_phyemud_phyemud_SOURCES = src/phyemud/main.c \
			src/hal/comm/comm_private.h \
			src/phyemud/manager.h src/phyemud/manager.c \
			src/phyemud/phyunix.c src/phyemud/physerial.c
src_phyemud_phyemud_LDADD = @GLIB_LIBS@
src_phyemud_phyemud_LDFLAGS = $(AM_LDFLAGS)
src_phyemud_phyemud_CFLAGS = $(AM_CFLAGS) @GLIB_CFLAGS@ \
				-I$(top_srcdir)/src \
				-I$(top_srcdir)/src/hal/comm

examples_uthingd_SOURCES = examples/uthing.c \
			src/phyemud/phyunix.c
examples_uthingd_LDFLAGS = $(AM_LDFLAGS)
examples_uthingd_CFLAGS = $(AM_CFLAGS) @GLIB_CFLAGS@ \
				-I$(top_srcdir)/src \
				-I$(top_srcdir)/src/hal/comm

tools_phyd_test_SOURCES = tools/phyd_test.c src/drivers/phy_driver.c
tools_phyd_test_LDADD = libs/libphy_driver.a \
				 libs/libnrf24l01.a libs/libspi.a \
                 @GLIB_LIBS@
tools_phyd_test_LDFLAGS = $(AM_LDFLAGS)
tools_phyd_test_CFLAGS = $(AM_CFLAGS) @GLIB_CFLAGS@ \
               -I$(top_srcdir)/src/drivers -I$(top_srcdir)/src/hal/comm \
               -I$(top_srcdir)/src/spi \
               -I$(top_srcdir)/src/nrf24l01


DISTCLEANFILES =

MAINTAINERCLEANFILES = Makefile.in \
	aclocal.m4 configure config.h.in config.sub config.guess \
	ltmain.sh depcomp compile missing install-sh

all-local:
	$(MKDIR_P) $(top_srcdir)/include/arduino && cp $(lib_ARDUINO) $(top_srcdir)/include/arduino

clean-local:
	$(RM) -r proxy/spiproxyd nrf/nrfd
