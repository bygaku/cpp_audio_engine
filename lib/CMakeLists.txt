file(GLOB SOURCE CONFIGURE_DEPENDS "./src/*.cpp")

# Create an library.
add_library(${LIBRARY_NAME}
        STATIC
        ${SOURCE}
)

target_include_directories(${LIBRARY_NAME}
        PUBLIC
        ./include
)

# Compiler options.
if(MSVC)
    target_link_libraries(${LIBRARY_NAME}
            mmdevapi
            ksuser
            ole32
            uuid
            advapi32
    )
    target_compile_definitions(${LIBRARY_NAME}
            PRIVATE
            UNICODE     # Unicode mode
            _UNICODE    # Multi-byte mode
            NOMINMAX
    )

    add_compile_options(
            /utf-8      # using UTF-8
            /W4         # warning level: 4
            /wd4819     # suppression of character encoding warnings
            /TP         # all src file written by cxx.
    )

    set_property(TARGET ${LIBRARY_NAME}
            PROPERTY
            MSVC_CHARACTER_SET "Unicode"
    )   # use Unicode.
endif()