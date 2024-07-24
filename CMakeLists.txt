remove_definitions(-DTRANSLATION_DOMAIN="okular")
add_definitions(-DTRANSLATION_DOMAIN="okular_poppler" -DGLM_FORCE_RIGHT_HANDED)

add_subdirectory( conf )

set(CMAKE_REQUIRED_LIBRARIES Poppler::Qt5 Qt5::Core Qt5::Gui)

include_directories(
   "${CMAKE_CURRENT_SOURCE_DIR}/../.." "3rdParty/poppler/qt5/src/" "3rdParty/poppler/poppler/" "3rdParty/poppler/" "/usr/include/tirpc"
)

include_directories("../Okular-v3d-Plugin-Code/src/Rendering" "../Okular-v3d-Plugin-Code/src/" "../Okular-v3d-Plugin-Code/src/V3dFile" "../Okular-v3d-Plugin-Code/dependencies/" "../Okular-v3d-Plugin-Code/dependencies/VulkanTools" "/usr/include/tirpc" "/home/benjaminb/kde/build/kparts/src/" "/home/benjaminb/kde/build/kparts/" "3rdParty/gzip-hpp/include" "/home/benjaminb/kde/src/okular/generators/Okular-v3d-Plugin-Code/3rdParty/V3D-Common" "/usr/include/poppler/qt5")

########### next target ###############

set(okularGenerator_poppler_PART_SRCS
   generator_pdf.cpp
   formfields.cpp
   annots.cpp
   pdfsignatureutils.cpp
   pdfsettingswidget.cpp
   imagescaling.cpp
   ../Okular-v3d-Plugin-Code/3rdParty/V3D-Common/Rendering/renderheadless.cpp
   ../Okular-v3d-Plugin-Code/3rdParty/V3D-Common/3rdParty/VulkanTools/VulkanTools.cpp
   ../Okular-v3d-Plugin-Code/3rdParty/V3D-Common/V3dFile/V3dFile.cpp
   ../Okular-v3d-Plugin-Code/3rdParty/V3D-Common/V3dFile/V3dObject.cpp
   ../Okular-v3d-Plugin-Code/3rdParty/V3D-Common/V3dFile/V3dObjects.cpp
   ../Okular-v3d-Plugin-Code/3rdParty/V3D-Common/V3dFile/V3dUtil.cpp
   ../Okular-v3d-Plugin-Code/3rdParty/V3D-Common/Utility/Arcball.cpp
   ../Okular-v3d-Plugin-Code/3rdParty/V3D-Common/Utility/ProtectedFunctionCaller.cpp
)

ki18n_wrap_ui(okularGenerator_poppler_PART_SRCS
   conf/pdfsettingswidget.ui
)

kconfig_add_kcfg_files(okularGenerator_poppler_PART_SRCS GENERATE_MOC conf/pdfsettings.kcfgc )

okular_add_generator(okularGenerator_poppler ${okularGenerator_poppler_PART_SRCS})

set(POPPLER_QT5_LIB /home/benjaminb/kde/src/okular/generators/Okular-v3d-Embeded-Plugin-Code/3rdParty/poppler/qt5/src/libpoppler-qt5.so)

#link_directories("/home/benjaminb/kde/src/okular/generators/Okular-v3d-Embeded-Plugin-Code/3rdParty/poppler/qt5/src/")
target_link_libraries(okularGenerator_poppler okularcore KF5::I18n KF5::Completion KF5::KIOWidgets Qt5::Xml vulkan tirpc z Poppler::Qt5)

message("========================= FINDING ===============================")

find_library(NAMES poppler-qt5)
message("=========================== DONE FINDING =============================")

########### autotests ###############

ecm_add_test(autotests/testimagescaling.cpp
    TEST_NAME "imageScalingTest"
    LINK_LIBRARIES Qt5::Test Qt5::Gui
)

########### install files ###############
install( FILES okularPoppler.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
install( PROGRAMS okularApplication_pdf.desktop org.kde.mobile.okular_pdf.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES org.kde.okular-poppler.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )
