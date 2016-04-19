INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs-sql REQUIRED db-util)

ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/.worldclock.db
		COMMAND sqlite3 ${CMAKE_CURRENT_BINARY_DIR}/.worldclock.db ".read ${CMAKE_CURRENT_SOURCE_DIR}/timezone.sql"
		COMMENT "Creating timezone database"
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/timezone.sql
)
ADD_CUSTOM_TARGET(timezone_db ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/.worldclock.db)

#install database files
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/.worldclock.db DESTINATION ${TZ_SYS_DB})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/.worldclock.db-journal DESTINATION ${TZ_SYS_DB})
