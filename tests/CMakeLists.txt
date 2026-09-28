find_package(Qt6 REQUIRED COMPONENTS Test)

# One executable per test class; each runs against its own temporary database.
function(wave_add_test name)
    qt_add_executable(${name} ${name}.cpp testdatabase.h)
    target_link_libraries(${name} PRIVATE waveordering_core Qt6::Test wave_warnings)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

wave_add_test(tst_money)
wave_add_test(tst_database)
wave_add_test(tst_cart)
wave_add_test(tst_orders)

# QML smoke tests: link the static QML plugin so `import WaveOrdering` works.
wave_add_test(tst_qml)
target_link_libraries(tst_qml PRIVATE waveordering_coreplugin Qt6::Quick Qt6::QuickControls2)
