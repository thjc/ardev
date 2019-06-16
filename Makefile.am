SUBDIRS = libthjc src util doc include 

userdoc: 
	doxygen ./doc/ardev.config

count:
	wc -l ./src/*.h ./src/*.cpp \
		./include/ardev/*.h \
		./util/aride/src/*.h ./util/aride/src/*.cpp \
		./util/calib/src/*.h ./util/calib/src/*.cpp \
		./util/config_manager/include/*.h ./util/config_manager/src/*.cpp \
		./util/player_module/*.cc
