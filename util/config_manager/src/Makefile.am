AM_CPPFLAGS = -Wall -g -fPIC -fexceptions -I$(top_builddir)/include -I$(top_srcdir)/include -I$(top_srcdir)/libthjc/include -I$(srcdir)/../include -DDEBUG @QT_CFLAGS@ @QTXML_CFLAGS@ @QTCORE_CFLAGS@ @PLAYER_CFLAGS@ @ARTOOLKITPLUS_CFLAGS@
MOC=moc

# create the pkg-config entry for the server headers
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ardev-cm.pc

EXTRA_DIST = ardev-cm.pc.in

lib_LTLIBRARIES = libardev-cm.la 

libardev_cm_la_MOC = cm_parameter.moc.cpp cm_parameter_ardev.moc.cpp

libardev_cm_la_SOURCES = cm_project.cpp \
	cm_registry.cpp \
	cm_objecthandler.cpp \
	cm_artoolkithandlers.cpp \
	cm_artoolkitplushandlers.cpp \
	cm_parameter.cpp \
	cm_playerhandlers.cpp \
	cm_opencvblobhandlers.cpp \
	cm_pluginhandler.cpp

BUILT_SOURCES = $(libardev_cm_la_MOC) 
nodist_libardev_cm_la_SOURCES = $(BUILT_SOURCES)

libardev_cm_la_LIBADD = -L$(top_builddir)/src -lardev @QT_LIBS@ @QTXML_LIBS@ @QTCORE_LIBS@ @PLAYER_LIBS@ @OPENCV_LIBS@ @ARTOOLKITPLUS_LIBS@ 
if HAVE_IMAGEMAGICK
libardev_cm_la_LIBADD += -lMagick++
endif

CLEANFILES = $(BUILT_SOURCES)

%.moc.cpp: ../include/%.h
	$(MOC) -o $@ $<
