set(SHTTPS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SHTTPS_SOURCES
    ${SHTTPS_DIR}/App.cpp
    ${SHTTPS_DIR}/Http.cpp
    ${SHTTPS_DIR}/HttpHeader.cpp
    ${SHTTPS_DIR}/HttpException.cpp
    ${SHTTPS_DIR}/Mime.cpp
    ${SHTTPS_DIR}/Socket.cpp
    ${SHTTPS_DIR}/Utils.cpp
    ${SHTTPS_DIR}/ConnectionHandler.cpp)

find_package(PkgConfig REQUIRED)
find_package(gflags REQUIRED)

pkg_check_modules(glog REQUIRED libglog)
set(needed_includes ${glog_INCLUDE_DIRS})
set(needed_libraries ${glog_LIBRARIES})

add_executable(shttps ${SHTTPS_SOURCES})
target_include_directories(shttps PUBLIC ${needed_includes})
target_link_libraries(shttps PUBLIC gflags ${needed_libraries})
