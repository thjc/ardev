AM_CPPFLAGS = -Wall -fPIC -I$(top_builddir)/include -I$(top_srcdir)/include -I$(top_srcdir)/libthjc/include -DDEBUG -g @PLAYER_CFLAGS@ @ARTOOLKITPLUS_CFLAGS@ @OPENCV_CFLAGS@ @IMAGEMAGICK_CFLAGS@ @DC1394_CFLAGS@ 

# create the pkg-config entry for the server headers
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ardev.pc

EXTRA_DIST = ardev.pc.in
CLEANFILES = ardev.pc

lib_LTLIBRARIES = libardev.la 

libardev_la_SOURCES = exception.cpp \
	core.cpp output_x11.cpp output_Manage.cpp capture_v4l.cpp \
	ffmpeg.cpp output_ffmpeg.cpp \
	render_b21r.cpp 3ds.h 3ds.cpp render_3ds.cpp \
	artoolkitplus.cpp \
	artoolkit.cpp 

if HAVE_IMAGEMAGICK
libardev_la_SOURCES += capture_file.cpp 
endif

if HAVE_PLAYER
libardev_la_SOURCES += capture_stage.cpp render_player_sonar.cpp render_player_laser.cpp \
	position_player.cpp render_player_path.cpp \
	render_player_ir.cpp player_core.cpp capture_player.cpp \
	render_player_bumper.cpp render_player_ptz.cpp \
	render_player_map.cpp render_player_localise.cpp \
	render_player_graphics2d.cpp render_player_graphics3d.cpp \
	render_player_actarray.cpp \
	render_player_limb.cpp
if HAVE_GEOS
libardev_la_SOURCES += render_player_vectormap.cpp
endif
endif

if HAVE_DC1394
libardev_la_SOURCES += capture_dc1394.cpp
endif

if HAVE_OPENCV
libardev_la_SOURCES += opencv_blobtrack.cpp 
endif

if HAVE_ANTTWEAKBAR
libardev_la_SOURCES += anttweakbar.cpp 
endif

libardev_la_DEPENDENCIES = ../include/ardev/ardev.h \
	../include/ardev/ardevconfig.h \
	../include/ardev/exception.h \
	../include/ardev/ffmpeg.h \
	../include/ardev/output_ffmpeg.h \
	../include/ardev/output_Manage.h \
	../include/ardev/capture.h \
	../include/ardev/player.h \
	../include/ardev/render_base.h \
	../include/ardev/artoolkit.h \
	../include/ardev/opencv_blobtrack.h \
	../include/ardev/anttweakbar.h

if HAVE_PLAYER
noinst_PROGRAMS = test 
test_SOURCES = test.cpp
test_LDFLAGS = -L. -L$(top_builddir)/libthjc/src
test_LDADD = -lardev -lthjc @DC1394_LIBS@ @OPENCV_LIBS@ @ARTOOLKITPLUS_LIBS@ @IMAGEMAGICK_LIBS@ @PLAYER_LIBS@
endif
