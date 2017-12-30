
set(SRC_LIST imgui_bgfx.cpp)
set(HDR_LIST imgui_bgfx.h  )

add_library(ws_imgui_impl ${SRC_LIST} ${HDR_LIST})
target_link_libraries(ws_imgui_impl
    PUBLIC
        external::bgfx
        imgui::imgui
)

target_enable_lto(ws_imgui_impl optimized)

target_set_warnings(ws_imgui_impl
    ENABLE ALL
    AS_ERROR ALL
    DISABLE Annoying
)

add_library(bgfx::imgui ALIAS ws_imgui_impl)