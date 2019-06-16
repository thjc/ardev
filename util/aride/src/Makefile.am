AM_CPPFLAGS = -Wall -g -fPIC -fexceptions -I$(top_builddir)/include -I$(top_srcdir)/include -I$(top_srcdir)/libthjc/include -I$(srcdir)/../../config_manager/include -DDEBUG @QT_CFLAGS@ @QTXML_CFLAGS@ @QTCORE_CFLAGS@ @PLAYER_CFLAGS@ @ARTOOLKITPLUS_CFLAGS@
noinst_LIBRARIES = libaride_src.a

MOC=moc

libaride_src_a_MOC = mainwindow.moc.cpp ConfigureEnvironments.moc.cpp ConfigureDisplaylist.moc.cpp

BUILT_SOURCES = $(libaride_src_a_MOC) 

libaride_src_a_SOURCES = main.cpp mainwindow.h mainwindow.cpp \
	ConfigureEnvironments.h ConfigureEnvironments.cpp \
	ConfigureDisplaylist.h ConfigureDisplaylist.cpp

nodist_libaride_src_a_SOURCES = $(BUILT_SOURCES)

CLEANFILES = $(BUILT_SOURCES)

%.moc.cpp: %.h
	$(MOC) -o $@ $<
