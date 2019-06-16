AM_CPPFLAGS = -Wall -I$(srcdir) -I$(top_builddir)/include -I$(top_srcdir)/include -I$(top_srcdir)/libthjc/include -DDEBUG @QT_CFLAGS@ @QTCORE_CFLAGS@ @QTCOMPAT_CFLAGS@ @PLAYER_CFLAGS@ @OPENCV_CFLAGS@ @IMAGEMAGICK_CFLAGS@
noinst_LIBRARIES = libcalib_src.a

MOC = moc

libcalib_src_a_MOC = calibapp.moc.cpp PixmapWidget.moc.cpp 

BUILT_SOURCES = $(libcalib_src_a_MOC) 

libcalib_src_a_SOURCES = main.cpp calibapp.h calibapp.cpp \
	hough.h hough.cpp \
	camera.h camera.cpp \
	PixmapWidget.h PixmapWidget.cpp

nodist_libcalib_src_a_SOURCES = $(BUILT_SOURCES)

CLEANFILES = $(BUILT_SOURCES)

%.moc.cpp: %.h
	$(MOC) -o $@ $<
