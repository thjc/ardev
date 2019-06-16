AM_CPPFLAGS = -g -Wall -fPIC -I$(top_srcdir)/include -DDEBUG @QT_CFLAGS@ @QTXML_CFLAGS@
noinst_LIBRARIES = libaride_ui.a

EXTRA_DIST=mainwindow.ui configuredisplaylist.ui configureenvironments.ui

MOC=moc
UIC=uic

#libaride_ui_a_MOC = mainwindow_base.moc.cpp configureenvironments_base.moc.cpp \
#					configuredisplaylist_base.moc.cpp
libaride_ui_a_UI = mainwindow_base.h \
					configureenvironments_base.h \
					configuredisplaylist_base.h 
#libaride_ui_a_UI = mainwindow_base.h mainwindow_base.cpp \
#					configureenvironments_base.h configureenvironments_base.cpp \
#					configuredisplaylist_base.h configuredisplaylist_base.cpp

mainwindow_base.h: mainwindow.ui
mainwindow_base.cpp: mainwindow.ui mainwindow_base.h

configureenvironments_base.h: configureenvironments.ui
configureenvironments_base.cpp: configureenvironments.ui configureenvironments_base.h

configuredisplaylist_base.h: configuredisplaylist.ui
configuredisplaylist_base.cpp: configuredisplaylist.ui configuredisplaylist_base.h


libaride_ui_a_SOURCES =

BUILT_SOURCES = $(libaride_ui_a_MOC) $(libaride_ui_a_UI) 
CLEANFILES = $(BUILT_SOURCES)
nodist_libaride_ui_a_SOURCES = $(BUILT_SOURCES)

%_base.moc.cpp: %_base.h
	$(MOC) -o $@ $<

%_base.h: %.ui
	$(UIC) -o $@ $<

#%_base.cpp: %.ui 
#	$(UIC) -o $@ -impl $*_base.h $<
