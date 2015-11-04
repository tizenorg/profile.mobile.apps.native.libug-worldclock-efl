SET(LIBRARY_NAME ${LIB_NAME_COM})
#lib
SET(LIB_SRCS
        src/clock_fwk_util.c
        src/clock_fwk_icu.c
)

#set define
SET(ALARMRING_PKGNAME ${PKGNAME_RING})

# dependency
INCLUDE(FindPkgConfig)
pkg_check_modules(clk_com_pkgs REQUIRED
                       elementary appcore-efl
                       gobject-2.0 gmodule-2.0 dlog
                       vconf
                       icu-i18n
                       ecore-imf ecore ecore-file ecore-input
)

# CFlags
FOREACH(flag ${clk_com_pkgs_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC ${EXTRA_CFLAGS}")
ADD_DEFINITIONS("-fpic")
ADD_DEFINITIONS("-DDEBUG")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")
ADD_DEFINITIONS("-DALARMRING_PKGNAME=\"${PKGNAME_RING}\"")

IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
    MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

#lib
ADD_LIBRARY(${LIBRARY_NAME} ${LIB_SRCS})
SET_TARGET_PROPERTIES(${LIBRARY_NAME} PROPERTIES SOVERSION ${SO_VERSION_MAJOR})
SET_TARGET_PROPERTIES(${LIBRARY_NAME} PROPERTIES VERSION ${SO_VERSION})
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} ${clk_com_pkgs_LDFLAGS})

#lib
INSTALL(TARGETS ${LIBRARY_NAME} DESTINATION ${LIBDIR})
