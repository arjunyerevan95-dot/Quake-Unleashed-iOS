# Initial native game target. Retail assets are deliberately not bundled.
target_sources(ironwail PRIVATE
    ports/ios/game/sdl_zink_bridge.c ports/ios/game/ios_bootstrap.m)
target_compile_definitions(ironwail PRIVATE QVR_IOS WITHOUT_CURL)
target_compile_options(ironwail PRIVATE
    -include ${CMAKE_CURRENT_SOURCE_DIR}/ports/ios/game/sdl_zink_bridge.h)
# Bridge implementation must call real SDL functions, not its own macro aliases.
set_source_files_properties(ports/ios/game/sdl_zink_bridge.c PROPERTIES
    COMPILE_DEFINITIONS QVR_BRIDGE_IMPL)
target_link_libraries(ironwail PRIVATE
    ${QVR_NATIVE_INPUTS}/libOSMesa.dylib
    "-framework Foundation" "-framework UIKit" "-framework Metal"
    "-framework QuartzCore" "-framework CoreGraphics")
if(TARGET SDL2::SDL2main)
    target_link_libraries(ironwail PRIVATE SDL2::SDL2main)
elseif(TARGET SDL2main)
    target_link_libraries(ironwail PRIVATE SDL2main)
endif()
set_target_properties(ironwail PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/ports/ios/game/Info.plist
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED NO
    BUILD_RPATH "@executable_path/Frameworks"
    INSTALL_RPATH "@executable_path/Frameworks")
