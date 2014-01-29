set(TARGET_DIR ${PROJECT_SOURCE_DIR}/xml)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(lxgui-xml STATIC
    ${SRCROOT}/xml_block.cpp
    ${SRCROOT}/xml_document.cpp
)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/xml*.hpp)
install(FILES ${files} DESTINATION include/lxgui)
install(TARGETS lxgui-xml ARCHIVE DESTINATION lib)

