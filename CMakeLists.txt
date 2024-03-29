cmake_minimum_required(VERSION 3.0)
project(sq)

############# PCAP #############
set(PCAP_INCLUDE "C:/Users/ning/Desktop/WpdPack/Include")
set(PCAP_LIB C:/Users/ning/Desktop/WpdPack/Lib/wpcap.lib)
include_directories(${PCAP_INCLUDE})
################################

############# Tins #############
set(TINS_INCLUDE C:/Users/ning/Desktop/libtins-master/include)
set(TINS_DEBUG C:/Users/ning/Desktop/libtins-master/build/lib/Debug/tins.lib)
set(TINS_RELEASE C:/Users/ning/Desktop/libtins-master/build/lib/Release/tins.lib)
include_directories(${TINS_INCLUDE})
################################

############# ZLib #############
set(ZLIB_INCLUDE C:/Users/ning/Desktop/zlib-1.2.8/include)
set(ZLIB_DEBUG C:/Users/ning/Desktop/zlib-1.2.8/build/Debug/zlibstaticd.lib)
set(ZLIB_RELEASE C:/Users/ning/Desktop/zlib-1.2.8/build/Release/zlibstatic.lib)
include_directories(${ZLIB_INCLUDE})
################################

########## Http-Parser #########
set(HTTP_PARSER_INCLUDE C:/Users/ning/Desktop/http-parser-master)
set(HTTP_PARSER_DEBUG C:/Users/ning/Desktop/http-parser-master/build/Debug/http_parser.lib)
set(HTTP_PARSER_RELEASE C:/Users/ning/Desktop/http-parser-master/build/Release/http_parser.lib)
include_directories(${HTTP_PARSER_INCLUDE})
################################

############# MD5 ##############
set(MD5_INCLUDE C:/Users/ning/Desktop/md5)
set(MD5_DEBUG C:/Users/ning/Desktop/md5/build/Debug/md5.lib)
set(MD5_RELEASE C:/Users/ning/Desktop/md5/build/Release/md5.lib)
include_directories(${MD5_INCLUDE})
################################

############## Boost ###########
set(BOOST_INCLUDE C:/local/boost_1_57_0)
set(BOOST_LIB_DIR C:/local/boost_1_57_0/lib32-msvc-12.0)
set(BOOST_DEBUG "${BOOST_LIB_DIR}/libboost_filesystem-vc120-mt-gd-1_57.lib" 
"${BOOST_LIB_DIR}/libboost_system-vc120-mt-gd-1_57.lib")

set(BOOST_RELEASE "${BOOST_LIB_DIR}/libboost_filesystem-vc120-mt-1_57.lib" 
"${BOOST_LIB_DIR}/libboost_system-vc120-mt-1_57.lib")
include_directories(${BOOST_INCLUDE})
################################

############## Lua ##############
set(LUA_DIR C:/Users/ning/Desktop/lua-5.3.0)
set(LUA_INCLUDE ${LUA_DIR}/src)
set(LUA_LIB_DIR ${LUA_DIR}/build/src)
set(LUA_DEBUG ${LUA_LIB_DIR}/Debug/liblua.lib)
set(LUA_RELEASE ${LUA_LIB_DIR}/Release/liblua.lib)
include_directories(${LUA_INCLUDE})
################################

########## C++ Format ##########
# Spdlog has already included C++ Format
# set(CPPFORMAT_DIR C:\Users\ning\Desktop\cppformat-master)
# set(CPPFORMAT_INCLUDE )
################################

############ POCO ##############
set(POCO_DIR "C:/Users/ning/Desktop/poco-1.6.0")
set(POCO_FOUNDATION_INCLUDE "${POCO_DIR}/Foundation/include")
set(POCO_NET_INCLUDE "${POCO_DIR}/Net/include")
include_directories("${POCO_FOUNDATION_INCLUDE}" "${POCO_NET_INCLUDE}")

set(POCO_LIB_DIR "C:/Users/ning/Desktop/poco-1.6.0/lib")

set(POCO_FOUNDATION_LIB_DEBUG "${POCO_LIB_DIR}/PocoFoundationmdd.lib")
set(POCO_FOUNDATION_LIB_RELEASE "${POCO_LIB_DIR}/PocoFoundationmd.lib")
set(POCO_NET_LIB_DEBUG "${POCO_LIB_DIR}/PocoNetmdd.lib")
set(POCO_NET_LIB_RELEASE "${POCO_LIB_DIR}/PocoNetmd.lib")

set(POCO_DEBUG "${POCO_FOUNDATION_LIB_DEBUG}" "${POCO_NET_LIB_DEBUG}")
set(POCO_RELEASE "${POCO_FOUNDATION_LIB_RELEASE}" "${POCO_NET_LIB_RELEASE}")

add_definitions(-DPOCO_STATIC)
################################

include_directories("C:/usr/include"
                    src)

# Main Sources 
set(LIB_SRC src/sniffer.cpp
            src/sniffer.hpp
            src/stream.cpp
            src/stream.hpp
            src/streamid.hpp
            src/type.cpp
            src/type.hpp
            src/server.cpp
            src/server.hpp
            src/proxy.cpp
            src/proxy.hpp
            src/url.cpp
            src/url.hpp
            src/storage.hpp
            src/storage.cpp            
            src/parser.cpp
            src/parser.hpp
            src/sniffer_stream.cpp
            src/sniffer_stream.hpp
            src/serialization.cpp
            src/serialization.hpp
            src/one.cpp
            src/one.hpp
            src/utility.hpp
            src/utility.cpp)

###### Project ######
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS) # -DBOOST_ALL_NO_LIB)

# Main Dependencies
set(DEBUG_LIB     "${TINS_DEBUG}" 
                  "${ZLIB_DEBUG}" 
                  "${HTTP_PARSER_DEBUG}" 
                  "${MD5_DEBUG}"
                  "${BOOST_DEBUG}"
                  "${LUA_DEBUG}"
                  "${POCO_DEBUG}")

set(RELEASE_LIB   "${TINS_RELEASE}" 
                  "${ZLIB_RELEASE}" 
                  "${HTTP_PARSER_RELEASE}" 
                  "${MD5_RELEASE}"
                  "${BOOST_RELEASE}"
                  "${LUA_RELEASE}"
                  "${POCO_RELEASE}")

set(DEBUG_RELEASE_LIB Iphlpapi "${PCAP_LIB}" ws2_32)

# Main Library
add_library(sqlib ${LIB_SRC})
target_link_libraries(sqlib debug "${DEBUG_LIB}" )
target_link_libraries(sqlib optimized "${RELEASE_LIB}")

# Main Executable
add_executable(sq src/main.cpp)
target_link_libraries(sq debug "${DEBUG_LIB}")
target_link_libraries(sq optimized "${RELEASE_LIB}")
target_link_libraries(sq "${DEBUG_RELEASE_LIB}" sqlib)

# Python Wrapper
add_library(sq_ext SHARED src/sq_ext.cpp)
target_link_libraries(sq_ext debug "${DEBUG_LIB}")
target_link_libraries(sq_ext optimized "${RELEASE_LIB}")
target_link_libraries(sq_ext "${DEBUG_RELEASE_LIB}" sqlib)

###### Test ######

# Unit Test
# Test Dependencies
set(GMOCK_INCLUDE C:/Users/ning/Desktop/googlemock/include)
set(GTEST_INCLUDE C:/Users/ning/Desktop/googlemock/gtest/include)
set(GMOCK_LIB_DIR C:/Users/ning/Desktop/googlemock/build)
set(GMOCK_DEBUG ${GMOCK_LIB_DIR}/Debug/gmock.lib)
set(GMOCK_RELEASE ${GMOCK_LIB_DIR}/Release/gmock.lib)

include_directories(
  ${GMOCK_INCLUDE}
  ${GTEST_INCLUDE})

set(UNIT_TEST_SRC test/test_main.cpp
                  test/test_storage.cpp
                  test/test_proxy.cpp
                  test/test_parser.cpp
                  test/test_stream.cpp
                  test/test_url.cpp
                  test/test_serialization.cpp
				  test/mock_storage.hpp)

add_executable(sq_unittest ${UNIT_TEST_SRC})
target_link_libraries(sq_unittest debug "${DEBUG_LIB}" "${GMOCK_DEBUG}")
target_link_libraries(sq_unittest optimized "${RELEASE_LIB}" "${GMOCK_RELEASE}")
target_link_libraries(sq_unittest "${DEBUG_RELEASE_LIB}" sqlib)

# Integration Test

add_executable(sniffer_stream test/test_sniffer_stream.cpp)
target_link_libraries(sniffer_stream debug "${DEBUG_LIB}")
target_link_libraries(sniffer_stream optimized "${RELEASE_LIB}")
target_link_libraries(sniffer_stream "${DEBUG_RELEASE_LIB}" sqlib)

add_executable(cache test/test_cache.cpp)
target_link_libraries(cache debug "${DEBUG_LIB}")
target_link_libraries(cache optimized "${RELEASE_LIB}")
target_link_libraries(cache "${DEBUG_RELEASE_LIB}" sqlib)

add_executable(server test/test_server.cpp)
target_link_libraries(server debug "${DEBUG_LIB}")
target_link_libraries(server optimized "${RELEASE_LIB}")
target_link_libraries(server "${DEBUG_RELEASE_LIB}" sqlib)
