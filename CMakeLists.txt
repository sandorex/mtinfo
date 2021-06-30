# Copyright 2020-2021 Aleksandar Radivojević
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

cmake_minimum_required (VERSION 3.15)

project (mtinfo VERSION 0.1.0
                DESCRIPTION "Modern terminfo library"
                LANGUAGES CXX)

set (mtinfo_SOVERSION 1)

include (cmake/StandardProjectSettings.cmake)
include (cmake/PreventInSourceBuilds.cmake)

add_library (project_options INTERFACE)
target_compile_features (project_options INTERFACE cxx_std_17)

if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    option (ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
    if (ENABLE_BUILD_WITH_TIME_TRACE)
        add_compile_definitions (project_options INTERFACE -ftime-trace)
    endif ()
endif ()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library (project_warnings INTERFACE)

# enable cache system
include (cmake/Cache.cmake)

# standard compiler warnings
include (cmake/CompilerWarnings.cmake)
set_project_warnings (project_warnings)

# sanitizer options if supported by compiler
include (cmake/Sanitizers.cmake)
enable_sanitizers (project_options)

# enable doxygen
include (cmake/Doxygen.cmake)
enable_doxygen ()

# allow for static analysis options
include (cmake/StaticAnalyzers.cmake)

option (BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option (BUILD_TESTS "Builds tests" ON)
option (BUILD_EXAMPLES "Build examples" ON)

configure_file (src/version.hh.in ${PROJECT_BINARY_DIR}/include/mtinfo/version.hh)
add_library (mtinfo src/terminfo/parser.cc)

target_link_libraries (mtinfo project_options project_warnings)
target_include_directories (mtinfo
    PUBLIC include/
    PUBLIC ${PROJECT_BINARY_DIR}/include/
    PRIVATE src/)
set_target_properties (mtinfo PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${mtinfo_SOVERSION}
    # PUBLIC_HEADER include/mtinfo/mtinfo.hh # TODO
    CXX_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_INLINES_HIDDEN ON)

include (GenerateExportHeader)
generate_export_header (mtinfo
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/mtinfo/export.hh)

if (BUILD_EXAMPLES)
    add_subdirectory (src/examples)
endif ()

if (BUILD_TESTS)
    add_subdirectory (tests)
endif ()
