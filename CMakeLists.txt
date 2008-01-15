# $Id$

SET(PACKAGE GD)
SET(PACKAGE_NAME GD)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.3 FATAL_ERROR)
PROJECT(GD)
SET(CPACK_PACKAGE_NAME "libgd")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "libGD, powerful and easy to use graphic library")
SET(CPACK_PACKAGE_VENDOR "http://www.libgd.org")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")

set(CPACK_GENERATOR TGZ)


set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_IGNORE_FILES
"~$"
"\\\\.swp$"
"\\\\.cvsignore$"
"^${PROJECT_SOURCE_DIR}.*/CVS/"
"^${PROJECT_SOURCE_DIR}/debian/"
"^${PROJECT_SOURCE_DIR}/old/"
"^${PROJECT_SOURCE_DIR}/bld/"
"^${PROJECT_SOURCE_DIR}/autom4te\\\\.cache/"
)

INCLUDE(CPack)
SET(CMAKE_MODULE_PATH "${GD_SOURCE_DIR}/cmake/modules")

if (BUILD_TEST)
	ENABLE_TESTING()
endif(BUILD_TEST)

if (NOT WIN32)
	STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
	IF (CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -O0")
	ENDIF (CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
endif (NOT WIN32)

if (USE_EXT_GD)
	message("Using GD at: ${USE_EXT_GD}")
	INCLUDE_DIRECTORIES(BEFORE ${GD_INCLUDE_DIR})
	FIND_PACKAGE(GD)
	if (GD_FOUND)
		INCLUDE_DIRECTORIES(BEFORE ${GD_INCLUDE_DIR})
		LINK_DIRECTORIES(BEFORE ${GD_LIBRARIES})
		SET(GD_LIB ${GD_LIBRARIES})
		SET(GD_LIBS_DIR ${GD_LIBRARY})

		message("GD libs #: ${GD_LIBRARIES}")
		message("GD lib #: ${GD_LIBRARY}")
		message("GD include: ${GD_INCLUDE_DIR}")
	else (GD_FOUND)
		message("No gd found")
	endif (GD_FOUND)
else (USE_EXT_GD)

	SET(GD_VERSION_MAJOR "2")
	SET(GD_VERSION_MINOR "1")
	SET(GD_VERSION_PATCH "0")
	SET(GD_VERSION "${GD_VERSION_MAJOR}.${GD_VERSION_MINOR}.${GD_VERSION_PATCH}")
	SET(GD_VERSION_STRING "${GD_VERSION}")

	SET(GD_VERSION_INT "20100")

	SET(CMAKE_REQUIRED_INCLUDES "/usr/include" "/usr/local/include")

	message(STATUS "ft2build: ${CMAKE_REQUIRED_INCLUDES}")

	include(CheckIncludeFiles)
	include(CheckIncludeFile)

	include(AC_HEADER_STDC)
	include(CheckPrototypeExists)
	SET(FONTCONFIG_FIND_QUIETLY, 1)

	if (ENABLE_PNG)
		FIND_PACKAGE(PNG REQUIRED)
	endif (ENABLE_PNG)

	FIND_PACKAGE(ZLIB)

	if (ENABLE_JPEG)
		FIND_PACKAGE(JPEG)
	endif (ENABLE_JPEG)

	if (ENABLE_TIFF)
		FIND_PACKAGE(TIFF)
	endif (ENABLE_TIFF)

	if (NOT WIN32)
		FIND_PACKAGE(PTHREAD)
	endif (NOT WIN32)

	if (ENABLE_FREETYPE)
		FIND_PACKAGE(Freetype)
	endif (ENABLE_FREETYPE)

	if (ENABLE_XPM)
		FIND_PACKAGE(XPM)
	endif (ENABLE_XPM)

	if (ENABLE_FONTCONFIG)
		FIND_PACKAGE(Fontconfig)
	endif (ENABLE_FONTCONFIG)



	if (FREETYPE_FOUND)
		INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIR})
		SET(HAVE_FT2BUILD_H 1)
		SET(HAVE_LIBFREETYPE 1)
	ENDIF(FREETYPE_FOUND)

	IF(ZLIB_FOUND)
		INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
		SET(HAVE_LIBZ 1)
		SET(HAVE_LIBPNG_PNG_H 1)
	ENDIF(ZLIB_FOUND)

	IF(PNG_FOUND)
		INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
		SET(HAVE_LIBPNG 1)
	ENDIF(PNG_FOUND)

	IF(XPM_FOUND)
		INCLUDE_DIRECTORIES(${XPM_INCLUDE_DIR})
		SET(HAVE_LIBXPM 1)
	ENDIF(XPM_FOUND)

	IF(JPEG_FOUND)
		INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
		SET(HAVE_LIBJPEG 1)
	ENDIF(JPEG_FOUND)

	IF(TIFF_FOUND)
		INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
		SET(HAVE_LIBTIFF 1)
	ENDIF(TIFF_FOUND)

	IF(FONTCONFIG_FOUND)
		INCLUDE_DIRECTORIES(${FONTCONFIG_INCLUDE_DIR})
		SET(HAVE_LIBFONTCONFIG 1)
	ELSE (FONTCONFIG_FOUND)
		SET(FONTCONFIG_LIBRARIES "")
		SET(FONTCONFIG_INCLUDE_DIR "")
		SET(FONTCONFIG_LIB_DIR "")
	ENDIF(FONTCONFIG_FOUND)

	SET(HAVE_CONFIG_H 1)

	ADD_DEFINITIONS(-DHAVE_CONFIG_H)

	CONFIGURE_FILE(${GD_SOURCE_DIR}/src/config.h.cmake ${CMAKE_BINARY_DIR}/config.h ESCAPE_QUOTES)

	set(BUILD_SHARED_LIBS On)

	if (WIN32)
		SET(GD_LIB bgd)
		ADD_DEFINITIONS( -DWIN32 -D_WIN32 -DMSWIN32 -DBGDWIN32 -DWINVER=0x0500  -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0600)
	else (WIN32)
		SET(GD_LIB gd)
	endif (WIN32)

	SET(GD_INCLUDE_DIR "${GD_SOURCE_DIR}/src" "${GD_SOURCE_DIR}")
	INCLUDE_DIRECTORIES(before ${FONTCONFIG_INCLUDE_DIR})

	add_subdirectory(src)
endif (USE_EXT_GD)

add_subdirectory(tests)
add_subdirectory(examples)
