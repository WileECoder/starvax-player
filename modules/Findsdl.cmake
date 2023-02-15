if (WIN32)
   set( SDL2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/modules/windows/SDL2/include")
   include_directories(${SDL2_INCLUDE_DIRS})

   find_library(SDL2_LIBRARY
       NAMES SDL2
       PATHS "${CMAKE_SOURCE_DIR}/modules/windows/SDL2/lib/x64"
       REQUIRED
   )
endif (WIN32)

if (UNIX)
    set( SDL2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/modules/linux/sdl2/include/SDL2)
    include_directories(${SDL2_INCLUDE_DIRS})

    find_library( SDL2_LIBRARY
        NAMES SDL2
        PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules/linux/sdl2/lib
        REQUIRED
    )
endif (UNIX)
