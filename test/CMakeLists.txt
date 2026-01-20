file(GLOB TEST_SRC CONFIGURE_DEPENDS "./*.cpp")
set(TESTING "test")

enable_testing()
include(FetchContent)
FetchContent_Declare(
        GoogleTest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(GoogleTest)

# Add test process in below.
add_executable(${TESTING}
        ${TEST_SRC}
)

# Add own library.
target_link_libraries(${TESTING}
        PRIVATE
        GTest::gtest
        GTest::gtest_main
        ${LIBRARY_NAME}
)

add_test(NAME t_audio_engine
        COMMAND
        $<TARGET_FILE:${TESTING}>
)

# Compiler options.
if(MSVC)
    target_compile_definitions(${TESTING}
            PRIVATE
            UNICODE     # Unicode mode
            _UNICODE    # Multi-byte mode
            NOMINMAX
    )

    set_property(TARGET ${TESTING}
            PROPERTY
            MSVC_CHARACTER_SET "Unicode"
    )   # use Unicode.

    add_compile_options(
            /utf-8      # using UTF-8
            /W4         # warning level: 4
            /wd4819     # suppression of character encoding warnings
            /TP         # all src file written by cxx.
    )
endif()
