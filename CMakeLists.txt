# Copyright 2020 Aleksandar Radivojević
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required (VERSION 3.10)

project (mtinfo VERSION 0.1.0
                DESCRIPTION "mtinfo library, also supports terminfo"
                LANGUAGES CXX)

# TODO add option to embed all the terminfo files selected or found on the system
# make an utility for this, make it pack all of it into an array for c and map
# for c++

# TODO sort out the flags
string (APPEND CMAKE_CXX_FLAGS_DEBUG " -Wall -Wextra -Wpedantic -ggdb3")
string (APPEND CMAKE_CXX_FLAGS_RELEASE " -Wall -Wextra -Wpedantic -O3")

configure_file (src/version.hh.in ${PROJECT_BINARY_DIR}/include/mtinfo/version.hh)

add_library (mtinfo src/cwrapper.cc
                    src/terminfo_parser.cc)
target_compile_features (mtinfo PUBLIC cxx_std_17)
target_include_directories (mtinfo
    PUBLIC include/
    PUBLIC ${PROJECT_BINARY_DIR}/include/
    PRIVATE src/)
set_target_properties (mtinfo PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    # PUBLIC_HEADER include/mtinfo/mtinfo.hh # TODO
    CXX_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_INLINES_HIDDEN ON)

include(GenerateExportHeader)
generate_export_header (mtinfo
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/mtinfo/export.hh)

add_subdirectory (tests)
