set(sdr_dso_SOURCES main.cc  dsobuffer.cc
    plotwidget.cc dsoctrlwidget.cc dsowidget.cc dso.cc dsoinputctrlwidget.cc
    dsotriggerctrlwidget.cc dsoplotctrlwidget.cc dsorecordctrlwidget.cc dsoaboutwidget.cc)
set(sdr_dso_MOC_HEADERS
    plotwidget.hh dsoctrlwidget.hh dsowidget.hh dso.hh dsoinputctrlwidget.hh
    dsotriggerctrlwidget.hh dsoplotctrlwidget.hh dsorecordctrlwidget.hh dsoaboutwidget.hh)

set(sdr_dso_HEADERS ${sdr_dso_MOC_HEADERS}
    dsobuffer.hh)
set(sdr_dso_RESOURCES "${CMAKE_SOURCE_DIR}/i18n/dso.qrc")

qt5_wrap_cpp(sdr_dso_MOC_SOURCES ${sdr_dso_MOC_HEADERS})

SET(LANGUAGES de_DE)
SET(LANGUAGE_TS_FILES)
FOREACH(LANGUAGE ${LANGUAGES})
  SET(TS_FILE "${CMAKE_SOURCE_DIR}/i18n/dso_${LANGUAGE}.ts")
  SET(LANGUAGE_TS_FILES ${LANGUAGE_TS_FILES} ${TS_FILE})
  SET_SOURCE_FILES_PROPERTIES(${TS_FILE} PROPERTIES OUTPUT_LOCATION
    "${CMAKE_SOURCE_DIR}/i18n")
ENDFOREACH(LANGUAGE ${LANGUAGES})
IF(LANGUAGE_TS_FILES)
  QT5_CREATE_TRANSLATION(sdr_dso_QM_FILES ${sdr_dso_SOURCES} ${LANGUAGE_TS_FILES})
ENDIF(LANGUAGE_TS_FILES)

QT5_ADD_RESOURCES(sdr_dso_RESOURCES_RCC ${sdr_dso_RESOURCES})

add_executable(dso WIN32 MACOSX_BUNDLE ${sdr_dso_SOURCES} ${sdr_dso_MOC_SOURCES}
                   ${sdr_dso_RESOURCES_RCC})
target_link_libraries(dso ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${LIBS} libsdr)

IF(WIN32 OR WIN64)
 install(TARGETS dso RUNTIME DESTINATION bin)
ENDIF(WIN32 OR WIN64)
