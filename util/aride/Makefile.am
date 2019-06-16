SUBDIRS = ui src res example

AM_CPPFLAGS = -Wall -fPIC -fexceptions -I$(top_srcdir)/include -DDEBUG -g 

bin_PROGRAMS = aride

aride_SOURCES = dummy.cpp
aride_LDFLAGS = -L./src/ -L./ui/ -L$(top_builddir)/src/ -L../config_manager/src/.libs -L$(top_builddir)/libthjc/src
aride_LDADD = -laride_src -laride_ui -lardev-cm -lardev -lthjc @QT_LIBS@ @QTXML_LIBS@ @QTCORE_LIBS@ @PLAYER_LIBS@ @OPENCV_LIBS@ @ARTOOLKITPLUS_LIBS@ @DC1394_LIBS@
if HAVE_IMAGEMAGICK
aride_LDADD += -lMagick++
endif
aride_DEPENDENCIES = ./src/libaride_src.a ./ui/libaride_ui.a ../config_manager/src/libardev-cm.la
