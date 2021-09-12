set(TARGET_DIR ${PROJECT_SOURCE_DIR}/xml)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-xml
    ${SRCROOT}/xml_block.cpp
    ${SRCROOT}/xml_document.cpp
)

# need C++17
target_compile_features(lxgui-xml PRIVATE cxx_std_17)

target_include_directories(lxgui-xml PUBLIC ${PROJECT_SOURCE_DIR}/include)

# needed dependencies
target_link_libraries(lxgui-xml PUBLIC lxgui-utils)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/xml*.hpp)
install(FILES ${files} DESTINATION include/lxgui)
install(TARGETS lxgui-xml ARCHIVE DESTINATION lib)

