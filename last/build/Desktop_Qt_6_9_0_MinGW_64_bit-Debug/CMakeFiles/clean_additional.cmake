# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\last_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\last_autogen.dir\\ParseCache.txt"
  "last_autogen"
  )
endif()
