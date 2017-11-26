
add_executable(ws_systemtest
    main.cpp
    FileStreamTests.cpp
    BitStreamTests.cpp
    SystemUtilsTests.cpp
)
target_link_libraries(ws_systemtest external::doctest WS::system)
set_target_properties(ws_systemtest PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/workingDirectory
)
add_test(
    NAME WS.systemtest
    COMMAND ws_systemtest ${TEST_RUNNER_PARAMS}
    WORKING_DIRECTORY $<TARGET_PROPERTY:ws_systemtest,VS_DEBUGGER_WORKING_DIRECTORY> # Would need to copy files if we install the tests ?
)
