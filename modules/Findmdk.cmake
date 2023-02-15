if (WIN32)

   include_directories(
      "${CMAKE_SOURCE_DIR}/modules/windows/mdk-sdk/include"
   )

   find_library(MDK_LIBRARY
       NAMES mdk
       PATHS "${CMAKE_SOURCE_DIR}/modules/windows/mdk-sdk/lib/x64"
       REQUIRED
   )

endif (WIN32)

if (UNIX)
   add_library(mdk SHARED IMPORTED)
   set_target_properties(mdk PROPERTIES
      IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/modules/linux/mdk-sdk/lib/amd64/libmdk.so)

   include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/modules/linux/mdk-sdk/include)

   set( MDK_LIBRARY mdk )
endif (UNIX)
