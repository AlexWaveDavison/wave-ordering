cmake_minimum_required(VERSION 3.21)

project(WaveOrdering
    VERSION 0.2.0
    DESCRIPTION "Wholesale ordering app built with Qt 6 / C++ / QML"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(WAVE_BUILD_TESTS "Build the unit tests" ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Sql)

qt_standard_project_setup(REQUIRES 6.5)

# Shared compiler warnings for everything we compile ourselves.
add_library(wave_warnings INTERFACE)
if(MSVC)
    target_compile_options(wave_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(wave_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Core library + QML module ("import WaveOrdering")
#
# All C++ logic and QML lives here so the app and the unit tests share it. It
# is built as a static library with a static QML plugin, which is what iOS
# requires anyway.
# ---------------------------------------------------------------------------
qt_add_library(waveordering_core STATIC)

target_sources(waveordering_core
    PRIVATE
        src/appcontroller.cpp src/appcontroller.h
        src/cartmodel.cpp src/cartmodel.h
        src/categorymodel.cpp src/categorymodel.h
        src/database.cpp src/database.h
        src/money.cpp src/money.h
        src/ordermodel.cpp src/ordermodel.h
        src/orderstore.cpp src/orderstore.h
        src/productmodel.cpp src/productmodel.h
        src/theme.h
)

qt_add_qml_module(waveordering_core
    URI WaveOrdering
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/Main.qml
        qml/pages/CatalogPage.qml
        qml/pages/CartPage.qml
        qml/pages/OrdersPage.qml
        qml/components/CartItemDelegate.qml
        qml/components/CategoryChip.qml
        qml/components/EmptyState.qml
        qml/components/OrderDetailDialog.qml
        qml/components/ProductCard.qml
        qml/components/ProductDetailDialog.qml
        qml/components/ProductTile.qml
        qml/components/QuantityStepper.qml
        qml/components/SendConfirmDialog.qml
        qml/components/StatusChip.qml
        qml/components/Toast.qml
    RESOURCES
        qml/icons/catalogue.png
        qml/icons/basket.png
        qml/icons/orders.png
        qml/icons/search.png
        qml/icons/logo.png
)

target_include_directories(waveordering_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(waveordering_core
    PUBLIC
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Sql
    PRIVATE
        wave_warnings
)

# ---------------------------------------------------------------------------
# App
# ---------------------------------------------------------------------------
qt_add_executable(appWaveOrdering app/main.cpp)

target_compile_definitions(appWaveOrdering PRIVATE WAVE_ORDERING_VERSION="${PROJECT_VERSION}")

target_link_libraries(appWaveOrdering
    PRIVATE
        waveordering_core
        waveordering_coreplugin
        Qt6::Gui
        Qt6::Quick
        Qt6::QuickControls2
        wave_warnings
)

set_target_properties(appWaveOrdering PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER com.waveordering.app
    MACOSX_BUNDLE_BUNDLE_NAME "Wave Ordering"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    WIN32_EXECUTABLE TRUE
)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(APP_ICON_ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/ios/Assets.xcassets)
    target_sources(appWaveOrdering PRIVATE ${APP_ICON_ASSETS})
    set_source_files_properties(${APP_ICON_ASSETS} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_target_properties(appWaveOrdering PROPERTIES
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
    )
endif()

include(GNUInstallDirs)
install(TARGETS appWaveOrdering
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(WAVE_BUILD_TESTS AND NOT ANDROID AND NOT IOS)
    enable_testing()
    add_subdirectory(tests)
endif()
