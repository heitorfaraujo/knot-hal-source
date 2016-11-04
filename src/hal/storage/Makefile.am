lib_LIBRARIES = libhalstorage.a
lib_ARDUINO = storage_avr.cpp

AM_CFLAGS = $(WARNING_CFLAGS) $(BUILD_CFLAGS)

libhalstorage_a_SOURCES = storage_linux.c
libhalstorage_a_CPPFLAGS = $(AM_CFLAGS) @GLIB_CFLAGS@
libhalstorage_a_DEPENDENCIES = $(top_srcdir)/include/storage.h

all-local:
	$(MKDIR_P) $(top_srcdir)/libs && cp $(lib_LIBRARIES) $(top_srcdir)/libs
	$(MKDIR_P) $(top_srcdir)/include/arduino && cp $(lib_ARDUINO) $(top_srcdir)/include/arduino

clean-local:
	$(RM) -r libhalstorage.a

