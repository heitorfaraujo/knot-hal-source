lib_LIBRARIES = libhaltime.a
lib_ARDUINO = time_avr.cpp

AM_CFLAGS = $(WARNING_CFLAGS) $(BUILD_CFLAGS)

libhaltime_a_SOURCES = time_linux.c
libhaltime_a_CPPFLAGS = $(AM_CFLAGS)
libhaltime_a_DEPENDENCIES = $(top_srcdir)/include/time.h

all-local:
	$(MKDIR_P) $(top_srcdir)/libs && cp $(lib_LIBRARIES) $(top_srcdir)/libs
	$(MKDIR_P) $(top_srcdir)/include/arduino && cp $(lib_ARDUINO) $(top_srcdir)/include/arduino

clean-local:
	$(RM) -r libhaltime.a
