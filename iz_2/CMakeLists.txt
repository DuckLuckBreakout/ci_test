cmake_minimum_required(VERSION 3.17)
set(PROJECT iz_2)
project(${PROJECT})
set(CMAKE_CXX_STANDARD 17)

################################
# Files
set (SOURCES
        )


##############################
# Release
set(RELEASE_FLAGS "-std=c++17 -Wall -pedantic")

# Debug
set(DEBUG_FLAGS "-std=c++17 -Wall -pedantic -g3")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")


#Choice target
if (CMAKE_BUILD_TYPE MATCHES Debug)
    message("BUILD_TYPE -- DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS}")
    add_executable (${PROJECT}
            Server.cpp
            ${SOURCES})

elseif (CMAKE_BUILD_TYPE MATCHES Test)
    message("BUILD_TYPE -- TEST")
    add_subdirectory(googletest)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RELEASE_FLAGS} -g -Wall")

    add_executable(${PROJECT}
            Tests/test.cpp
            Tests/unit_tests.cpp
            ${SOURCES})

    target_link_libraries(${PROJECT}
            gtest
            gtest_main
            )
    add_test(run_tests run_tests)

elseif (CMAKE_BUILD_TYPE MATCHES Release)
    message("BUILD_TYPE -- TEST")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RELEASE_FLAGS}")
    add_executable (${PROJECT}
            Server.cpp
            ${SOURCES})

endif ()
target_include_directories(${PROJECT} PRIVATE "${PROJECT_SOURCE_DIR}")
