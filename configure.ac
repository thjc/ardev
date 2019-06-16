dnl next few lines seem to be required for minimal usage
AC_INIT(src/core.cpp)
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE(ardev,0.10.2)
AM_CONFIG_HEADER(config.h include/ardev/ardevconfig.h)

dnl this is used by libtool to assign version numbers
ARDEV_VERSION_INFO="-version-number 0:10:0"
AC_SUBST(ARDEV_VERSION_INFO)


AC_PROG_LIBTOOL

dnl Process options

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

dnl check for pkg-config
dnl Do we have pkg-config?
AC_CHECK_PROG(have_pkg_config,pkg-config,yes,no)
if test "x$have_pkg_config" = "xno"; then
  AC_MSG_ERROR([pkg-config is required])
else
  dnl Add prefix to the pkg-config path in case RTK or Gazebo was
  dnl installed there and the user didn't add it manually .
  dnl
  dnl Also, add /usr/local/lib/pkgconfig, because on
  dnl some platforms, pkg-config won't look there by default.
  export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
  if test "x$prefix" != "xNONE"; then
    export PKG_CONFIG_PATH=$prefix/lib/pkgconfig:$PKG_CONFIG_PATH
  fi
fi

# Checks for libraries.
dnl Sort QT stuff
PKG_CHECK_MODULES(QT, QtGui >= 4.1, with_qt=yes, with_qt=no)
PKG_CHECK_MODULES(QTXML, QtXml >= 4.1, , with_qt=no)
PKG_CHECK_MODULES(QTCORE, QtCore >= 4.1, , with_qt=no)
PKG_CHECK_MODULES(QTCOMPAT, Qt3Support >= 4.1, , with_qt=no)

dnl ImageMagick
PKG_CHECK_MODULES(IMAGEMAGICK, ImageMagick++, with_imagemagick=yes, with_imagemagick=no)
AM_CONDITIONAL(HAVE_IMAGEMAGICK,test "x$with_imagemagick" = "xyes")

dnl std libs
AC_CHECK_LIB(m,sqrt,have_m=yes LIBS="$LIBS -lm")



dnl open gl etc
AC_CHECK_LIB(glut, main,have_glut=yes LIBS="$LIBS -lglut")
AC_CHECK_LIB(GLU, main,have_GLU=yes LIBS="$LIBS -lGLU")
AC_CHECK_HEADER(GL/gl.h, AC_CHECK_LIB(GL, glBegin,have_GL=yes LIBS="$LIBS -lGL"),)
AC_CHECK_LIB(Xinerama, XineramaIsActive)

dnl pthread
AC_CHECK_LIB(pthread, pthread_create,have_pthread=yes LIBS="$LIBS -lpthread")

dnl SDL
AC_CHECK_LIB(SDL,SDL_Init,have_SDL=yes LIBS="$LIBS -lSDL")

dnl SDL
AC_CHECK_LIB(ltdl,lt_dlinit,have_ltdl=yes LIBS="$LIBS -lltdl")

dnl dc1394
AC_ARG_ENABLE([dc1394],[  --disable-dc1394 to disable the ardev dc1394 capture module],[],[
	PKG_CHECK_MODULES(DC1394,libdc1394-2 >= 2.0.1,with_dc1394=yes AC_DEFINE(HAVE_DC1394,,libdc1394 is available),with_dc1394=no)
])
AM_CONDITIONAL(HAVE_DC1394,test "x$with_dc1394" = "xyes")

dnl V4L version 1
AC_CHECK_HEADERS(linux/videodev.h,have_v4l=yes)

dnl player
AC_ARG_ENABLE([player],[  --disable-player to disable the ardev player modules],[],[
  PLAYER_VER=0
  PKG_CHECK_MODULES(PLAYER, [playerc++ >= 2.2],
  [
    with_playercpp=yes
    AC_DEFINE(HAVE_PLAYER,,player is available)
    AC_DEFINE(PLAYER_VER,3,player is available)
  ],
  [
    PKG_CHECK_MODULES(PLAYER, [playerc++ >= 2.0],
    with_playercpp=yes
    AC_DEFINE(HAVE_PLAYER,,player is available)
    AC_DEFINE(PLAYER_VER,2,player is available)
    ,)
  ])
])
AM_CONDITIONAL(HAVE_PLAYER,test "x$with_playercpp" = "xyes")

dnl playercore
AC_ARG_ENABLE([playercore],[  --disable-playercore to disable the ardev player driver],[],[
PKG_CHECK_MODULES(PLAYERCORE,playercore >= 2,with_playercore=yes,with_playercore=no)
])

dnl ffmpeg
AC_ARG_ENABLE([ffmpeg],[  --enable-ffmpeg to enable the ffmpeg output options],[
AC_CHECK_LIB(avcodec,main,have_avcodec=yes LIBS="$LIBS -lavcodec",AC_MSG_ERROR([ffmpeg modules cannot build without libavcodec]))
AC_CHECK_LIB(avformat,main,have_avformat=yes LIBS="$LIBS -lavformat",AC_MSG_ERROR([ffmpeg modules cannot build without libavformat]))
AC_CHECK_LIB(avutil,main)
])
if test "x$have_avcodec" = "xyes" -a "x$have_avformat" = "xyes"; then
	AC_DEFINE(HAVE_FFMPEG,,[ffmpeg available] )
fi



dnl Needed for virtual face
AC_ARG_ENABLE([fep],[  --enable-fep to enable the virtual Virtual Face],[
AC_CHECK_LIB(fep,main, LIBS="$LIBS -lfep",AC_MSG_ERROR([FEP modules cannot build without libfep]))
AC_CHECK_LIB(TAO,main, LIBS="$LIBS -lTAO",AC_MSG_ERROR([FEP modules cannot build without libTAO]))
AC_CHECK_LIB(TAO_PortableServer,main, LIBS="$LIBS -lTAO_PortableServer",AC_MSG_ERROR([FEP modules cannot build without libTAO_PortableServer]))
AC_CHECK_LIB(TAO_Valuetype,main, LIBS="$LIBS -lTAO_Valuetype",AC_MSG_ERROR([FEP modules cannot build without libTAO_Valuetype]))
AC_CHECK_LIB(TAO_CosNaming,main, LIBS="$LIBS -lTAO_CosNaming",AC_MSG_ERROR([FEP modules cannot build without libTAO_CoseNaming]))
AC_CHECK_LIB(ACE,main, LIBS="$LIBS -lACE",AC_MSG_ERROR([FEP modules cannot build without libACE]))
AC_CHECK_LIB(xml2,main, LIBS="$LIBS -lxml2",AC_MSG_ERROR([FEP modules cannot build without libxml2]))
])

dnl artoolkit
AC_ARG_ENABLE([artoolkit],[  --enable-artoolkit to enable the artoolkit tracking modules],[
AC_CHECK_LIB(AR,main, LIBS="$LIBS -lAR" with_artoolkit=yes,AC_MSG_ERROR([ARToolkit modules cannot build without libAR]))
])
AM_CONDITIONAL(HAVE_ARTOOLKIT,test "x$with_artoolkit" = "xyes")

dnl artoolkitplus
AC_ARG_ENABLE([artoolkitplus],[  --disable-artoolkitplus to disable the ardev artoolkitplus modules],[],[
PKG_CHECK_MODULES(ARTOOLKITPLUS,artoolkitplus >= 2.2,with_artoolkitplus=yes AC_DEFINE(HAVE_ARTOOLKITPLUS,,artoolkitplus is available),with_artoolkitplus=no)
])
AM_CONDITIONAL(HAVE_ARTOOLKITPLUS,test "x$with_artookitplus" = "xyes")

dnl opencv
AC_ARG_ENABLE([opencv],[  --disable-opencv to disable the ardev opencv modules],[],[
PKG_CHECK_MODULES(OPENCV,opencv >= 0.9.7,with_opencv=yes AC_DEFINE(HAVE_OPENCV,,opencv is available),with_opencv=no)
])
AM_CONDITIONAL(HAVE_OPENCV,test "x$with_opencv" = "xyes")

dnl make sure we check for a function that is only in the 3.X api as we assume this to compile
AC_ARG_ENABLE([geos],[  --disable-geos to disable the ardev geos modules],[],[
AC_CHECK_LIB(geos_c,GEOSWKTReader_create,
             have_libgeos=yes
             AC_DEFINE(HAVE_GEOS, 1, [Define to 1 if you have the geos library (libgeos_c). ])
             AC_SUBST(GEOS_LIBS, -lgeos_c),
             have_libgeos=no)
])
AM_CONDITIONAL(HAVE_GEOS, test "x$have_libgeos" = "xyes")

dnl anttweakbar
AC_ARG_ENABLE([anttweakbar],[  --disable-anttweakbar to disable the ardev anttweakbar modules],[],[
AC_CHECK_LIB(AntTweakBar,main,have_anttweakbar=yes LIBS="$LIBS -lAntTweakBar -lGL",AC_MSG_ERROR([Extra GUI interactions will not build without AntTweakBar]))
])
AM_CONDITIONAL(HAVE_ANTTWEAKBAR, test "x$have_anttweakbar" = "xyes")

if test "x$have_anttweakbar" = "xyes" -a "x$have_anttweakbar" = "xyes"; then
	AC_DEFINE(HAVE_ANTTWEAKBAR,,[anttweakbar available] )
fi

# Checks for header files.
AC_PATH_X
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero floor gettimeofday memset pow sqrt strdup strerror])

# check which utils we should build
# Check for the recommended libs (utils will not be built without these
AC_SUBST(CALIB_DIR)
AC_SUBST(CM_LIBS)
AC_SUBST(ARIDE_DIR)
AC_SUBST(PLAYER_PLUGIN_DIR)
ARIDE_DIR=aride
CM_LIBS="-L../config_manager/src/.libs -lardev-cm"
CALIB_DIR=calib
PLAYER_PLUGIN_DIR=player_module
if test "x$with_qt" != "xyes"; then
	CALIB_DIR=
	ARIDE_DIR=
	CM_LIBS=
else
	AC_DEFINE(HAVE_ARIDE,,[We are building the aride util])
fi
if test "x$with_playercpp" != "xyes"; then
	CALIB_DIR=
fi
if test "x$with_imagemagick" != "xyes"; then
	CALIB_DIR=
fi
if test "x$with_opencv" != "xyes"; then
	CALIB_DIR=
fi
if test "x$with_playercore" != "xyes"; then
	PLAYER_PLUGIN_DIR=
fi


AC_ARG_ENABLE(optimize, [  --disable-optimize      Compile without optimization (O2) ],
    if test "x$enableval" = "xyes"; then
        CXXFLAGS="$CXXFLAGS -O2"
        CFLAGS="$CFLAGS -O2"
    elif test "x$enableval" = "xno"; then
        CXXFLAGS=`echo $CXXFLAGS | sed -e s/-O.//g`
        CFLAGS=`echo $CFLAGS | sed -e s/-O.//g`
    else
        CXXFLAGS="$CXXFLAGS -$enableval"
        CFLAGS="$CFLAGS -$enableval"
    fi,
)

AC_ARG_ENABLE(debug,    [  --disable-debug         Compile without debugging information (-g)],
    if test "x$enableval" = "xyes"; then
        CXXFLAGS="$CXXFLAGS -g"
        CFLAGS="$CFLAGS -g"
    elif test "x$enableval" = "xno"; then
        CXXFLAGS=`echo $CXXFLAGS | sed -e s/-g//g`
        CFLAGS=`echo $CFLAGS | sed -e s/-g//g`
    fi,
)

dnl Output Makefiles
AC_OUTPUT(Makefile
	libthjc/Makefile
	libthjc/src/Makefile
	libthjc/include/Makefile
	src/ardev.pc
	src/Makefile
	util/Makefile
	util/aride/Makefile
	util/aride/src/Makefile
	util/aride/ui/Makefile
	util/aride/example/Makefile
	util/aride/res/Makefile
	util/calib/Makefile
	util/calib/src/Makefile
	util/calib/ui/Makefile
	util/config_manager/src/ardev-cm.pc
	util/config_manager/Makefile
	util/config_manager/include/Makefile
	util/config_manager/src/Makefile
	util/player_module/Makefile
	doc/Makefile
	include/Makefile
	include/ardev/Makefile)



dnl pretty summary and warnings/errors re missing libraries
AC_MSG_RESULT([])
AC_MSG_RESULT([*************************************************************])
AC_MSG_RESULT([                  Configuration Summary                      ])
AC_MSG_RESULT([*************************************************************])
AC_MSG_RESULT([])

# Check the required libs
all_required="yes"
if test "x$have_m" != "xyes"; then
	AC_MSG_RESULT([	ARDev will not build without math library])
	all_required="no"
fi
if test "x$have_glut" != "xyes"; then
	AC_MSG_RESULT([	ARDev will not build without glut])
	all_required="no"
fi
if test "x$have_GLU" != "xyes"; then
	AC_MSG_RESULT([	ARDev will not build without GLU])
	all_required="no"
fi
if test "x$have_GL" != "xyes"; then
	AC_MSG_RESULT([	ARDev will not build without open GL])
	all_required="no"
fi
if test "x$have_pthread" != "xyes"; then
	AC_MSG_RESULT([	ARDev will not build without pthreads])
	all_required="no"
fi
if test "x$have_SDL" != "xyes"; then
	AC_MSG_RESULT([	ARDev will not build without SDL])
	all_required="no"
fi
if test "x$have_anttweakbar" != "xyes"; then
	AC_MSG_RESULT([	ARDev will not build without AntTweakBar])
	all_required="no"
fi
if test "x$have_ltdl" != "xyes"; then
	AC_MSG_RESULT([	ARDev will not build without the libtool ltdl library])
	all_required="no"
fi
if test "x$all_required" != "xyes"; then
	AC_MSG_RESULT([ *E* Missing required libraries, please correct this and then reconfigure])
	AC_MSG_ERROR([])
else
	AC_MSG_RESULT([ * * All Required libraries are present])
fi

# Check for the recommended libs (utils will not be built without these
all_recommended="yes"
if test "x$with_qt" != "xyes"; then
	AC_MSG_RESULT([ *W* Recommended library QT is missing])
	all_recommended="no"
fi
if test "x$have_anttweakbar" != "xyes"; then
	AC_MSG_RESULT([	*W* Recommended library AntTweakBar is missing])
	all_recommended="no"
fi
if test "x$with_playercpp" != "xyes"; then
	AC_MSG_RESULT([ *W* Recommended library Player is missing])
	all_recommended="no"
fi
if test "x$with_playercore" != "xyes"; then
	AC_MSG_RESULT([ *W* Recommended library Playercore is missing])
	all_recommended="no"
fi
if test "x$with_imagemagick" != "xyes"; then
	AC_MSG_RESULT([ *W* Recommended library imagemagick is missing])
	all_recommended="no"
fi
if test "x$with_artoolkitplus" != "xyes"; then
	AC_MSG_RESULT([ *W* Recommended library artoolkitplus is missing])
	all_recommended="no"
fi
if test "x$have_v4l" != "xyes"; then
	AC_MSG_RESULT([ *I* Building without V4L support])
fi
if test "x$all_recommended" != "xyes"; then
	AC_MSG_RESULT([ *W* Missing recommended libraries, many modules and utilities will not be available])
else
	AC_MSG_RESULT([ * * All recommended libraries are present])
fi

AC_MSG_RESULT([])
AC_MSG_RESULT([*************************************************************])
AC_MSG_RESULT([])

AC_MSG_RESULT([ARDev is now configured please run make])
