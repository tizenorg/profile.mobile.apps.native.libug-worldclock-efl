SET(SRCS
		worldclock_add_view.c
		worldclock_data.c
		worldclock_fwk_icu.c
		worldclock_util.c
		ug_worldclock_efl.c
		clock_fwk_icu_label.cpp
		taf.c
		taf-button.c
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
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DIMAGEDIR=\"${IMAGEDIR}\"")

SET(LIB_SO   -L${CMAKE_BINARY_DIR}/${COMMON_DIR} -l${LIB_NAME_COM})
#lib
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 0.1.0)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_SO})

ADD_DEPENDENCIES(${PROJECT_NAME} ${LIB_NAME_COM})

#install lib
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${UGLIBDIR})


# make worldclock.edj
ADD_CUSTOM_TARGET(ug_worldclock.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../res/images
		${CMAKE_CURRENT_SOURCE_DIR}/../res/edc/worldclock.edc ${CMAKE_BINARY_DIR}/ug_worldclock.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../res/edc/worldclock.edc
)



# make taf-button-theme.edj
ADD_CUSTOM_TARGET(ug_worldclock_button.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../res/images
		${CMAKE_CURRENT_SOURCE_DIR}/../res/edc/taf-button-theme.edc ${CMAKE_BINARY_DIR}/ug_worldclock_button.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../res/edc/taf-button-theme.edc
)

ADD_DEPENDENCIES(${PROJECT_NAME} ug_worldclock.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} ug_worldclock_button.edj)


# install edj file
INSTALL(FILES ${CMAKE_BINARY_DIR}/ug_worldclock.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/ug_worldclock_button.edj DESTINATION ${EDJDIR})
