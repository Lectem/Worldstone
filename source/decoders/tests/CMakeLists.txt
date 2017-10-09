
add_executable(ws_decoderstests decoderstests.cpp)
target_link_libraries(ws_decoderstests external::doctest WS::decoders)
set_target_properties(ws_decoderstests PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/workingDirectory
)

add_test(
    NAME WS.decoders
    COMMAND ws_decoderstests ${TEST_RUNNER_PARAMS}
    WORKING_DIRECTORY $<TARGET_PROPERTY:ws_decoderstests,VS_DEBUGGER_WORKING_DIRECTORY># Would need to copy files if we install the tests ?
)
