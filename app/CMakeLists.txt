# Create an executable file.
if(WIN32)
    add_executable(${EXE_NAME}
            WIN32
            ./main.cpp
    )
else()
    add_executable(${EXE_NAME}
            ./main.cpp
    )
endif()

target_link_libraries(${EXE_NAME}
        PRIVATE
        ${LIBRARY_NAME}
)

if(POP_CONSOLE) # Enable Popup Console Window
    target_compile_definitions(${EXE_NAME}
            PRIVATE
            POPUP_CONSOLE
    )
endif()

# Compiler options.
if(MSVC)
    target_compile_definitions(${EXE_NAME}
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

    set_property(TARGET ${EXE_NAME}
            PROPERTY
            MSVC_CHARACTER_SET "Unicode"
    )   # use Unicode.
endif()
