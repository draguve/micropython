# Create an INTERFACE library for our C module.
add_library(PicoNumber INTERFACE)

# Add our source files to the lib
target_sources(PicoNumber INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pico_num.c
    ${CMAKE_CURRENT_LIST_DIR}/q.c
    ${CMAKE_CURRENT_LIST_DIR}/m_string.c
)

# Add the current directory as an include directory.
target_include_directories(PicoNumber INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE PicoNumber)
