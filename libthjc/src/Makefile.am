AM_CPPFLAGS = -Wall -I$(srcdir)/../include -DDEBUG
lib_LTLIBRARIES = libthjc.la

libthjc_la_SOURCES = geometry.cpp matrix.cpp misc.cpp vector3d.cpp \
	logger.cpp logger_out_console.cpp serialcoms.cpp camera.cpp

LIBTOOL_DEPS = @LIBTOOL_DEPS@

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./configure.status --recheck
