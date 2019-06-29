project(WindowsService)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WIN32)
    set(SOURCES
        main.cpp
        json/json_reader.cpp
        json/json_value.cpp
        json/json_writer.cpp
        json/version.h.in
        json_db.cpp
    )
    file(GLOB_RECURSE h_list "*.h*")
    list(APPEND SOURCES ${h_list})

    #=================== Target =====================
    set(TARGET_NAME "WindowsService")
    add_executable(${TARGET_NAME} ${SOURCES})

    #================== Libraries ===================
    #target_link_libraries(${TARGET_NAME} XXX)
    install(TARGETS ${TARGET_NAME} DESTINATION bin)
    
endif(WIN32)
