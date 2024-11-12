cmake_minimum_required(VERSION 3.20)
project(Http_Server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(SOURCE_DIR ../Source)
set(MYSQL_SOURCE_DIR "C:\\Program Files\\MySQL\\MySQL Server 8.0")
set(RAPIDJSON_SOURCE_DIR "C:\\Program Files\\rapidjson")
set(JWT_CPP_SOURCE_DIR "C:\\Program Files\\jwt-cpp")
set(CURL_SOURCE_DIR "C:\\Program Files\\curl")

include_directories (   ${MYSQL_SOURCE_DIR}/include
                        ${RAPIDJSON_SOURCE_DIR}/include 
                        ${JWT_CPP_SOURCE_DIR}/include
                        ${CURL_SOURCE_DIR}/include      )

link_directories    (   ${MYSQL_SOURCE_DIR}/lib
                        ${CURL_SOURCE_DIR}/lib  )

link_libraries      (   ws2_32
                        mysql
                        libcurl
                        libcrypto )

add_compile_options (   -Wall 
                        -Wextra
                        -pedantic
                        -g          )

add_executable  (   http-server  
                    ${SOURCE_DIR}/Server.cpp
                    ${SOURCE_DIR}/Database_Handler.cpp
                    ${SOURCE_DIR}/JSON_Handler.cpp
                    ${SOURCE_DIR}/Auth_Handler.cpp
                    ${SOURCE_DIR}/CGPT_Handler.cpp      )

set(CMAKE_VERBOSE_MAKEFILE OFF)