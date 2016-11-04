lib_LIBRARIES = libhallog.a
lib_ARDUINO = log_avr.cpp

AM_CFLAGS = $(WARNING_CFLAGS) $(BUILD_CFLAGS)

libhallog_a_SOURCES = log_linux.c
libhallog_a_DEPENDENCIES = $(top_srcdir)/include/log.h

all-local:
	$(MKDIR_P) $(top_srcdir)/libs && cp $(lib_LIBRARIES) $(top_srcdir)/libs
	$(MKDIR_P) $(top_srcdir)/include/arduino && cp $(lib_ARDUINO) $(top_srcdir)/include/arduino
clean-local:
	$(RM) -r libhallog.a
