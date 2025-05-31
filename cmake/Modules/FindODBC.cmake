# cmake/Modules/FindODBC.cmake
message(STATUS "Loading custom FindODBC.cmake")  # 用于验证是否加载此文件

find_path(ODBC_INCLUDE_DIR
  NAMES sql.h
  PATHS /usr/include /usr/local/include
)

find_library(ODBC_LIBRARY
  NAMES odbc
  PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ODBC
  REQUIRED_VARS ODBC_LIBRARY ODBC_INCLUDE_DIR
)

if(ODBC_FOUND)
  set(ODBC_LIBRARIES ${ODBC_LIBRARY})
  set(ODBC_INCLUDE_DIRS ${ODBC_INCLUDE_DIR})
  if(NOT TARGET ODBC::ODBC)
    add_library(ODBC::ODBC UNKNOWN IMPORTED)
    set_target_properties(ODBC::ODBC PROPERTIES
      IMPORTED_LOCATION "${ODBC_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${ODBC_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(ODBC_INCLUDE_DIR ODBC_LIBRARY)