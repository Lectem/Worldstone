
add_executable(ws_systemtest systemtests.cpp)
target_link_libraries(ws_systemtest doctest WS::system)

add_test(
    NAME WS.systemtest
    COMMAND ws_systemtest ${TEST_RUNNER_PARAMS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/workingDirectory # Would need to copy files if we install the tests ?
)
