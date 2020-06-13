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

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

string (APPEND CMAKE_CXX_FLAGS_DEBUG " -Wall -Wpedantic -fexceptions -Og -g3 -ggdb")
string (APPEND CMAKE_CXX_FLAGS_RELEASE " -Wall -Wpedantic -fexceptions -O3")

add_library (mtinfo src/terminfo.cc
                    src/terminfo_utils.cc)
target_compile_features (mtinfo PUBLIC cxx_std_17)
target_include_directories (mtinfo
    PUBLIC include/
    PUBLIC ${PROJECT_BINARY_DIR}
    PRIVATE src/)
set_target_properties (mtinfo PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    # PUBLIC_HEADER include/mtinfo/mtinfo.hh # TODO
    CXX_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_INLINES_HIDDEN ON)

include(GenerateExportHeader)
generate_export_header (mtinfo)

add_subdirectory (tests)
