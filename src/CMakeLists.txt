INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # For generated files
  ${CMAKE_CURRENT_SOURCE_DIR} # FIXME: Get rid of cross header dependencies between /src and /include
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${ALSA_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIR}
  ${CAIROMM_INCLUDE_DIRS}
  ${CDIO_INCLUDE_DIRS}
  ${CDPARANOIA_INCLUDE_DIRS}
  ${CLUTTERMM_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${GDL_INCLUDE_DIRS}
  ${GIOMM_INCLUDE_DIRS}
  ${GLADEMM_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${GST_INCLUDE_DIRS}
  ${GTKGLEXT_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
  ${HAL_INCLUDE_DIRS}
  ${NM_INCLUDE_DIRS}
  ${MPD_INCLUDE_DIRS}
  ${LIBOFA_INCLUDE_DIRS}
  ${PYGTK_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_PATH}
  ${RSVG_INCLUDE_DIRS}
  ${SIGX_INCLUDE_DIRS}
  ${SOUP_INCLUDE_DIRS}
  ${SQLITE_INCLUDE_DIRS}
  ${STARTUP_NOTIFY_INCLUDE_DIRS}
  ${TAGLIB_INCLUDE_DIRS}
  ${X11_X11_INCLUDE_PATH}
  ${XML_INCLUDE_DIRS}
  ${XPROTO_INCLUDE_DIRS}
  ${rimokon_add_INCLUDE_DIRS}
)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mpx-app-dbus-adaptor.hh
  COMMAND ${DBUSBINDINGTOOL} ARGS "${PROJECT_SOURCE_DIR}/dbus/dbus-obj-YOUKI.xml" "--adaptor=${CMAKE_CURRENT_BINARY_DIR}/mpx-app-dbus-adaptor.hh"
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/dbus/dbus-obj-YOUKI.xml
)

SET(rimokon_SOURCES
  artist-images.cc
  covers.cc
  covers-stores.cc
  glib-marshalers.h
  glib-marshalers.c
  kobo-position.hh
  kobo-position.cc
  kobo-volume.hh
  kobo-volume.cc
  youki-theme-engine.hh
  youki-theme-engine.cc
  minisoup.cc
  network.cc
  mpxpd.cc
  mpxpd-sql.cc
  mpxpd-mpd.cc
  mpxpd-mpd.hh
  mpxpd-shell.hh
  mpxpd-shell.cc
  plugin.cc
  plugin-loader-cpp.hh
  plugin-loader-cpp.cc
  plugin-manager-gui.hh
  plugin-manager-gui.cc
  preferences.hh
  preferences.cc
  util.cc
)

# FIXME: Add definitions from: ${GLIB_CFLAGS} and ${GST_CFLAGS}

LIST(APPEND rimokon_COMPILE_DEFINITIONS __USE_POSIX199309)

IF(HAVE_LINUX)
  LIST(APPEND rimokon_COMPILE_DEFINITIONS _XOPEN_SOURCE=600)
ENDIF(HAVE_LINUX)

ADD_EXECUTABLE(rimokon ${rimokon_SOURCES})

SET_TARGET_PROPERTIES(rimokon
  PROPERTIES
  COMPILE_DEFINITIONS "${rimokon_COMPILE_DEFINITIONS}"
  LINK_FLAGS "-Wl,--export-dynamic"
)

TARGET_LINK_LIBRARIES(rimokon
  ${ALSA_LIBRARIES}
  ${Boost_REGEX_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_IOSTREAMS_LIBRARY}
  ${Boost_PYTHON_LIBRARY}
  ${CAIROMM_LIBRARIES}
  ${CLUTTERMM_LIBRARIES}
  ${CDIO_LIBRARIES}
  ${CDPARANOIA_LIBRARY}
  ${CURL_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GDL_LIBRARIES}
  ${GIOMM_LIBRARIES}
  ${GLADEMM_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${GST_LIBRARIES}
  ${GTKGLEXT_LIBRARIES}
  ${GTKMM_LIBRARIES}
  ${HAL_LIBRARIES}
  ${NM_LIBRARIES}
  ${LIBOFA_LIBRARIES}
  ${MPD_LIBRARIES}
  ${PYGTK_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${RSVG_LIBRARIES}
  ${SEXYMM_LIBRARIES}
  ${SIGX_LIBRARIES}
  ${SM_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${SQLITE_LIBRARIES}
  ${STARTUP_NOTIFY_LIBRARIES}
  ${TAGLIB_LIBRARIES}
  ${XML_LIBRARIES}
  ${X11_X11_LIB}
  jnetlib
  mconfig
  mpx-api
  mpx-widgets
  mpx-musicbrainz
  mpx-view-components
  mpx-json
  ${rimokon_ADD_LIBRARIES}
)

INSTALL(TARGETS rimokon RUNTIME DESTINATION ${MPX_BINDIR})
