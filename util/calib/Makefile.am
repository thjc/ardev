SUBDIRS = ui src

AM_CPPFLAGS = -Wall -I$(top_srcdir)/include -I$(top_srcdir)/libthjc/include -DDEBUG

bin_PROGRAMS = ardev-calib

ardev_calib_SOURCES = dummy.cpp
ardev_calib_LDFLAGS = -L./src/ -L./ui/ -L$(top_builddir)/src/ -L$(top_builddir)/libthjc/src
ardev_calib_LDADD = -lthjc -lcalib_src -lcalib_ui -lardev -lthjc @QT_LIBS@ @QTCORE_LIBS@ @PLAYER_LIBS@ @QTCOMPAT_LIBS@ @OPENCV_LIBS@ @ARTOOLKITPLUS_LIBS@ @DC1394_LIBS@ @IMAGEMAGICK_LIBS@
ardev_calib_DEPENDENCIES = ./src/libcalib_src.a ./ui/libcalib_ui.a 
