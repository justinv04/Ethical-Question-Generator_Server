cmake_minimum_required(VERSION 3.20)
project(Http_Server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(BACKEND_DIR ../Source/Backend)
set(MYSQL_SOURCE_DIR "C:\\Program Files\\MySQL\\MySQL Server 8.0")
set(RAPIDJSON_SOURCE_DIR "C:\\Program Files\\rapidjson")
set(OPENSSL_SOURCE_DIR "C:\\Program Files\\openssl") 
set(JWT_CPP_SOURCE_DIR "C:\\Program Files\\jwt-cpp")

include_directories( ${MYSQL_SOURCE_DIR}/include
                     ${RAPIDJSON_SOURCE_DIR}/include 
                     ${OPENSSL_SOURCE_DIR}/include
                     ${JWT_CPP_SOURCE_DIR}/include
)

link_directories(   ${MYSQL_SOURCE_DIR}/lib
                    ${OPENSSL_SOURCE_DIR}/bin
)

link_libraries(     ws2_32
                    libmysql.dll
                    libssl-3-x64.dll
                    libcrypto-3-x64.dll
)

add_compile_options(-Wall -Wextra -pedantic -g)

add_executable(http-server  ${BACKEND_DIR}/Server.cpp
                            ${BACKEND_DIR}/Database_Handler.cpp
                            ${BACKEND_DIR}/JSON_Handler.cpp
                            ${BACKEND_DIR}/Auth_Handler.cpp
)

set(CMAKE_VERBOSE_MAKEFILE OFF)