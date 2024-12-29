# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/esp/esp-idf/components/bootloader/subproject"
  "D:/DEVELOPER_SHIT/Clean-25/Redford/build/bootloader"
  "D:/DEVELOPER_SHIT/Clean-25/Redford/build/bootloader-prefix"
  "D:/DEVELOPER_SHIT/Clean-25/Redford/build/bootloader-prefix/tmp"
  "D:/DEVELOPER_SHIT/Clean-25/Redford/build/bootloader-prefix/src/bootloader-stamp"
  "D:/DEVELOPER_SHIT/Clean-25/Redford/build/bootloader-prefix/src"
  "D:/DEVELOPER_SHIT/Clean-25/Redford/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/DEVELOPER_SHIT/Clean-25/Redford/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
