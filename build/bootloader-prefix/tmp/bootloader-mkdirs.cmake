# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/deedee/esp/esp-idf_5_3/components/bootloader/subproject"
  "/Users/deedee/lcd/build/bootloader"
  "/Users/deedee/lcd/build/bootloader-prefix"
  "/Users/deedee/lcd/build/bootloader-prefix/tmp"
  "/Users/deedee/lcd/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/deedee/lcd/build/bootloader-prefix/src"
  "/Users/deedee/lcd/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/deedee/lcd/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/deedee/lcd/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
