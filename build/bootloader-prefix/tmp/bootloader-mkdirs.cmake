# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/thatg/esp/esp-idf/components/bootloader/subproject"
  "D:/DevShit/NEw/ESP/Redford/build/bootloader"
  "D:/DevShit/NEw/ESP/Redford/build/bootloader-prefix"
  "D:/DevShit/NEw/ESP/Redford/build/bootloader-prefix/tmp"
  "D:/DevShit/NEw/ESP/Redford/build/bootloader-prefix/src/bootloader-stamp"
  "D:/DevShit/NEw/ESP/Redford/build/bootloader-prefix/src"
  "D:/DevShit/NEw/ESP/Redford/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/DevShit/NEw/ESP/Redford/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
