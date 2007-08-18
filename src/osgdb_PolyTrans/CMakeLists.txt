SET( LIB_NAME osgdb_PolyTrans )

SET( COMHELPER_SRC_DIR "${osgOQ_SOURCE_DIR}/src/COMHelper" )
SET( COMHELPER_INC_DIR "${osgOQ_SOURCE_DIR}/include/COMHelper" )


SET( LIB_PUBLIC_HEADERS
    ${COMHELPER_INC_DIR}/PolyTransComHelper.h
    ${COMHELPER_INC_DIR}/PolyTransCOMSinkEvents.h
    ${COMHELPER_INC_DIR}/PolyTransImporter.h
    ${COMHELPER_INC_DIR}/PolyTransOutput.h
    ${COMHELPER_INC_DIR}/PolyTransOutputConsole.h
    ${COMHELPER_INC_DIR}/shared_okino_com_src.h
    ${COMHELPER_INC_DIR}/shared_okino_com_src_resources.h
    ${COMHELPER_INC_DIR}/stdafx.h
)

ADD_LIBRARY( ${LIB_NAME} MODULE
    ${LIB_PUBLIC_HEADERS}
    osgdb_PolyTrans.cpp
    ${osgOQ_SOURCE_DIR}/src/osgOQ/ConfigFileReader.cpp
    ${COMHELPER_SRC_DIR}/PolyTransComHelper.cpp
    ${COMHELPER_SRC_DIR}/PolyTransCOMSinkEvents.cpp
    ${COMHELPER_SRC_DIR}/PolyTransImporter.cpp
    ${COMHELPER_SRC_DIR}/PolyTransOutput.cpp
    ${COMHELPER_SRC_DIR}/PolyTransOutputConsole.cpp
    ${COMHELPER_SRC_DIR}/shared_okino_com_src_batch.cpp
    ${COMHELPER_SRC_DIR}/shared_okino_com_src_main.cpp
)

TARGET_LINK_LIBRARIES( ${LIB_NAME}
    comctl32.lib
)
LINK_INTERNAL( ${LIB_NAME}
    osgOQ
    osgDB
    osgUtil
    osg
    OpenThreads
)
