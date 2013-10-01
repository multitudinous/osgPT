cmake_minimum_required (VERSION 2.8.8)

project( osgpt )

set(osgpt_MAJOR_VERSION 0)
set(osgpt_MINOR_VERSION 1)
set(osgpt_SUB_VERSION 0)
set(osgpt_VERSION ${osgpt_MAJOR_VERSION}.${osgpt_MINOR_VERSION}.${osgpt_SUB_VERSION})
set(osgpt_PACKAGE_VERSION ${osgpt_VERSION})

set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )

#Set the include directory so that squirrel can be used as a sub project
if( NOT(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME}) )
    set(osgpt_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" PARENT_SCOPE)
    set(osgpt_LIBRARIES "osgpt" PARENT_SCOPE)
endif()

# Platform specific definitions
IF(WIN32)
    IF(MSVC)
        # To enable /MP, parralel build on MSVC
        OPTION(WIN32_USE_MP "Set to OFF to diable /MP." ON)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        IF(WIN32_USE_MP)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ENDIF(WIN32_USE_MP)

        # Other MSVC compilation flags
        #ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
        #ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)

    ENDIF(MSVC)
    if(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
        add_definitions(/W2)
    endif()
ENDIF (WIN32)

if(CMAKE_COMPILER_IS_GNUCXX)
    include(GNUInstallDirs)
endif(CMAKE_COMPILER_IS_GNUCXX)

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib)
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)

set(osgpt_INSTALL_INCDIR include)
set(osgpt_INSTALL_BINDIR bin)
set(osgpt_INSTALL_ARCHIVEDIR ${CMAKE_INSTALL_LIBDIR})
if(WIN32)
    SET(osgpt_INSTALL_LIBDIR bin)
else(WIN32)
    SET(osgpt_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
endif(WIN32)

# On windows provide the user with the pdb files 
# for debugging if they are present
if(MSVC)
    install(
        DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/\${CMAKE_INSTALL_CONFIG_NAME}/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        USE_SOURCE_PERMISSIONS
        COMPONENT development
        FILES_MATCHING PATTERN "*.pdb" )
endif(MSVC)

include_directories( ${PROJECT_SOURCE_DIR} )

set( OKINO_SDK_ROOT "Please specify" CACHE PATH "Root directory where the Okino SDK is installed" )

include( FindOSGHelper )

############################################################
# Compile the squirrel library

file( GLOB osgpt_srcs "*.cpp" )
file( GLOB osgpt_hdrs "*.h" )

list( APPEND osgpt_srcs ${osgpt_hdrs} )
list( APPEND osgpt_srcs EXP_ISU_OSG.RC)

include_directories( ${OSG_INCLUDE_DIRS} ${OKINO_SDK_ROOT}/includes )

add_library(exp_isu_osg SHARED ${osgpt_srcs} )

set_target_properties( exp_isu_osg PROPERTIES PROJECT_LABEL "Lib exp_isu_osg" )

set_source_files_properties(EXP_ISU_OSG.RC PROPERTIES LANGUAGE RC)

target_link_libraries( exp_isu_osg  ${OSG_LIBRARIES} comctl32.lib )
if( ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11 Win64" )
    target_link_libraries( exp_isu_osg  ${OKINO_SDK_ROOT}/libs/vc8_x64/vc64_imp.lib )
    target_link_libraries( exp_isu_osg  ${OKINO_SDK_ROOT}/libs/vc8_x64/vc64_nu.lib )
else()
    target_link_libraries( exp_isu_osg  ${OKINO_SDK_ROOT}/libs/vc8_x86/vc4_imp.lib )
    target_link_libraries( exp_isu_osg  ${OKINO_SDK_ROOT}/libs/vc8_x86/vc4_nu.lib )
endif()
set_target_properties( exp_isu_osg PROPERTIES VERSION ${osgpt_VERSION} )
#if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#    set_target_properties(squirrel PROPERTIES COMPILE_FLAGS "-fno-exceptions -fno-rtti -fno-strict-aliasing" )
#endif(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

source_group( "Header Files" FILES ${osgpt_hdrs})

install(TARGETS exp_isu_osg
    EXPORT osgpt-targets
    RUNTIME DESTINATION ${osgpt_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${osgpt_INSTALL_LIBDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${osgpt_INSTALL_ARCHIVEDIR} COMPONENT development)

install(FILES ${PROJECT_SOURCE_DIR}/EXP_ISU_OSG.HLP 
              ${PROJECT_SOURCE_DIR}/EXP_ISU_OSG.RTI
        DESTINATION ${osgpt_INSTALL_BINDIR}
        COMPONENT runtime)

############################################################

############################################################

#
# Create and install all of the auto find tools
# =============================================
set(VES_EXPORT_LIBRARY_TARGETS exp_isu_osg )
# Add all targets to the build-tree export set
if( EXISTS "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}LibraryDepends.cmake" )
    export(TARGETS exp_isu_osg APPEND
        FILE "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}LibraryDepends.cmake")
else()
    export(TARGETS exp_isu_osg
        FILE "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}LibraryDepends.cmake")
endif()

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE ${PROJECT_NAME})

# Create a squirrelBuildTreeSettings.cmake file for the use from the build tree
configure_file(${PROJECT_SOURCE_DIR}/CMakeModules/packageBuildTreeSettings.cmake.in
    "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}BuildTreeSettings.cmake" @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/CMakeModules/packageConfigVersion.cmake.in
    "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/CMakeModules/Usepackage.cmake.in
    "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/Use${PROJECT_NAME}.cmake" @ONLY)

include(CMakePackageConfigHelpers)
# Create the squirrelConfig.cmake and squirrelConfigVersion files
configure_package_config_file( 
    ${PROJECT_SOURCE_DIR}/CMakeModules/packageConfig.cmake.in 
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PATH_VARS CMAKE_INSTALL_LIBDIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}Config.cmake 
              ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}ConfigVersion.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/Use${PROJECT_NAME}.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT development)

# Install the CMake find script for storyteller
configure_file(${PROJECT_SOURCE_DIR}/CMakeModules/Findpackage.cmake.in
    "${PROJECT_BINARY_DIR}/Find${PROJECT_NAME}.cmake" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/Find${PROJECT_NAME}.cmake"
    DESTINATION "share/${PROJECT_NAME}/extra"
    COMPONENT development)
    #RENAME Find${PROJECT_NAME}.cmake)

# Install the export set for use with the install-tree
install(EXPORT ${PROJECT_NAME}-targets DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development)

