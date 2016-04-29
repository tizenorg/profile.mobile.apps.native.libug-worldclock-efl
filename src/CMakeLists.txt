SET(SRCS
		worldclock_add_view.c
		worldclock_data.c
		worldclock_fwk_icu.c
		worldclock_util.c
		ug_worldclock_efl.c
		clock_fwk_icu_label.cpp
	)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	appcore-efl
	elementary
	db-util
	dlog
	vconf
	ui-gadget-1
	bundle
	icu-i18n
	efl-extension
	capi-system-system-settings
	tapi
	notification
	libtzplatform-config
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} -fvisibility=hidden -Wall -Werror")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")
SET(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})

IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DIMAGEDIR=\"${IMAGEDIR}\"")

#bin
ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

#install
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})

# make worldclock.edj
ADD_CUSTOM_TARGET(worldclock.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../edje/images
		${CMAKE_CURRENT_SOURCE_DIR}/../res/edje/worldclock.edc ${CMAKE_BINARY_DIR}/worldclock.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../res/edje/worldclock.edc
)

ADD_DEPENDENCIES(${PROJECT_NAME} worldclock.edj)

# install edj file
INSTALL(FILES ${CMAKE_BINARY_DIR}/worldclock.edj DESTINATION ${EDJDIR})
