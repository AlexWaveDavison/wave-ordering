cmake_minimum_required(VERSION 3.16)

project(WaveOrdering VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Sql QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appWaveOrdering
    src/main.cpp
    src/database.h
    src/database.cpp
    src/categorymodel.h
    src/categorymodel.cpp
    src/productmodel.h
    src/productmodel.cpp
    src/cartmodel.h
    src/cartmodel.cpp
    src/ordermodel.h
    src/ordermodel.cpp
    src/appcontroller.h
    src/appcontroller.cpp
)

qt_add_qml_module(appWaveOrdering
    URI WaveOrdering
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/pages/CatalogPage.qml
        qml/pages/CartPage.qml
        qml/pages/OrdersPage.qml
        qml/components/ProductCard.qml
        qml/components/CategoryChip.qml
        qml/components/CartItemDelegate.qml
        qml/components/QuantityStepper.qml
        qml/components/ProductDetailDialog.qml
        qml/components/OrderDetailDialog.qml
)

set_target_properties(appWaveOrdering PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER com.waveordering.app
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appWaveOrdering
    PRIVATE
        Qt6::Quick
        Qt6::Sql
        Qt6::QuickControls2
)

include(GNUInstallDirs)
install(TARGETS appWaveOrdering
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
