
# define use_cxx11 in a way that's compatible with older cmake versions.
macro(use_cxx11)
    set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
endmacro(use_cxx11)

add_executable(rate_test 
    src/rate_test.cpp
    src/logging.cpp
    src/ipvs_interface.cpp
    src/net_tools.cpp
    depend/libipvs/libipvs.c
    depend/libipvs/ip_vs_nl_policy.c
)

add_executable(ipvshttpd
    src/ipvshttpd.cpp
    src/logging.cpp
    src/configuration.cpp
    src/ipvs_interface.cpp
    src/net_tools.cpp
    src/string_tools.cpp
    src/file_tools.cpp
    src/http_server.cpp
    depend/libipvs/libipvs.c
    depend/libipvs/ip_vs_nl_policy.c
)

set(CPPNETLIB_INCLUDE_DIRS 
    "${CMAKE_SOURCE_DIR}/depend/cpp-netlib"
    "${CMAKE_SOURCE_DIR}/depend/cpp-netlib/deps/uri/include"
)
set(CPPNETLIB_LIBS 
    "${CMAKE_SOURCE_DIR}/depend/cpp-netlib/build/libs/network/src/libcppnetlib-server-parsers.a"
)

set(Boost_USE_STATIC_LIBS ON)
#find_package(Boost 1.41 REQUIRED COMPONENTS system thread program_options)
find_package(Boost 1.41 REQUIRED COMPONENTS system)
set(JSONCPP_INCLUDE_DIR "/usr/include/jsoncpp")
find_library(
    JSONCPP_LIBRARY
    NAMES jsoncpp
    DOC "jsoncpp library"
)

MESSAGE("jsoncpp include dir = ${JSONCPP_INCLUDE_DIR}")
MESSAGE("jsoncpp library = ${JSONCPP_LIBRARY}")


include_directories("${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/depend" "${CPPNETLIB_INCLUDE_DIRS}" 
    "${Boost_INCLUDE_DIRS}" "${Boost_INCLUDE_DIRS}/boost" "${JSONCPP_INCLUDE_DIR}")

target_link_libraries(ipvshttpd "${CPPNETLIB_LIBS}" "${Boost_LIBRARIES}" 
    "${Boost_SYSTEM_LIBRARY}" "${Boost_THREAD_LIBRARY}" "${JSONCPP_LIBRARY}" "pthread" "ssl" "crypto")
use_cxx11()
