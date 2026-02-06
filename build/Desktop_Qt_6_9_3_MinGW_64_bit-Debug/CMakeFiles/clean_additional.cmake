# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\SE_KG_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SE_KG_autogen.dir\\ParseCache.txt"
  "SE_KG_autogen"
  )
endif()
