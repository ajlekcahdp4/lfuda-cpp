set (LFUDA_RESOURCES
    src/main.cc
)

add_executable (lfudacpp ${LFUDA_RESOURCES})
target_include_directories(lfudacpp PUBLIC ${LFUDA_INCLUDE_DIR})


if (BASH_PROGRAM)
    add_test(NAME TestLFU-DA.TestEndToEnd COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh "$<TARGET_FILE:lfudacpp>" ${CMAKE_CURRENT_SOURCE_DIR})
endif()
