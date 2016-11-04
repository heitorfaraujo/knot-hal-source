lib_LIBRARIES = libhalcomm.a
lib_ARDUINO = comm_nrf24l01.c comm_private.h

AM_CFLAGS = $(WARNING_CFLAGS) $(BUILD_CFLAGS)

libhalcomm_a_SOURCES = comm_nrf24l01.c
libhalcomm_a_CPPFLAGS = $(AM_CFLAGS) -I$(top_srcdir)/src/nrf24l01 -I$(top_srcdir)/include
libhalcomm_a_DEPENDENCIES = $(top_srcdir)/include/comm.h

all-local:
	$(MKDIR_P) $(top_srcdir)/libs && cp $(lib_LIBRARIES) $(top_srcdir)/libs
	$(MKDIR_P) $(top_srcdir)/include/arduino && cp $(lib_ARDUINO) $(top_srcdir)/include/arduino
clean-local:
	$(RM) -r libhalcomm.a
