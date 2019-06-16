AM_CPPFLAGS = -Wall -I../src/ -I$(top_srcdir)/include -DDEBUG 
noinst_LIBRARIES = libcalib_ui.a
EXTRA_DIST=calibapp.ui

MOC=moc
UIC=uic

libcalib_ui_a_UI = calibapp_base.h

calibapp_base.h: calibapp.ui
calibapp_base.cpp: calibapp.ui calibapp_base.h

libcalib_ui_a_SOURCES =
nodist_libcalib_ui_a_SOURCES = $(libcalib_ui_a_MOC) $(libcalib_ui_a_UI) 

BUILT_SOURCES = $(libcalib_ui_a_MOC) $(libcalib_ui_a_UI) 
CLEANFILES = $(BUILT_SOURCES)

%_base.moc.cpp: %_base.h
	$(MOC) -o $@ $<

%_base.h: %.ui
	$(UIC) -o $@ $<

