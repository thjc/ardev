AM_CPPFLAGS = -Wall -g -fPIC -I$(top_builddir)/include -I$(top_srcdir)/include -I$(top_srcdir)/libthjc/include -I$(srcdir)/../config_manager/include @PLAYERCORE_CFLAGS@ @PLAYER_CFLAGS@ @OPENCV_CFLAGS@ @ARTOOLKITPLUS_CFLAGS@  @QT_CFLAGS@ @QTXML_CFLAGS@ @QTCORE_CFLAGS@ @DC1394_CFLAGS@

lib_LTLIBRARIES = libardev-player-plugin.la 
libardev_player_plugin_la_SOURCES = ardev_plugin.cc
libardev_player_plugin_la_LIBADD = -L$(top_builddir)/src/ -L$(top_builddir)/libthjc/src/ -lthjc -lardev @PLAYER_LIBS@ @OPENCV_LIBS@ @ARTOOLKITPLUS_LIBS@ @CM_LIBS@ @QT_LIBS@ @QTXML_LIBS@ @QTCORE_LIBS@ @DC1394_LIBS@
libardev_player_plugin_la_DEPS = $(top_builddir)/src/libardev.a
libardev_player_plugin_la_DEPENDENCIES = 
