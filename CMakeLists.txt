file(GLOB SRC CONFIGURE_DEPENDS *.cpp)

add_library(Base STATIC ${SRC})
target_include_directories(Base PUBLIC .)
target_include_directories(Base PRIVATE 3rdparty/stb-master)
target_link_libraries(Base PRIVATE d2d1.lib)
target_compile_options(Base PRIVATE -exceptions)