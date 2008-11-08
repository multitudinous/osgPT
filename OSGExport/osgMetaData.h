
#ifndef __OSG_META_DATA_H__
#define __OSG_META_DATA_H__


#include "main.h"
#include <osg/Vec3>
#include <string>
#include <map>

namespace osg {
    class Node;
    class LOD;
}

class MetaDataCollector
{
public:
    MetaDataCollector( Nd_Token Nv_Handle_Type, char *Nv_Handle_Name );
    ~MetaDataCollector();

    void add( std::string& name, std::string& value );

    void configureLOD( osg::LOD* lod );

    // Is the meta data item name available?
    bool hasMetaData( const std::string& name );

    // Retrieve meta data values
    bool getMetaData( const std::string& name, std::string& ret );
    bool getMetaData( const std::string& name, double& ret );
    bool getMetaData( const std::string& name, osg::Vec3& ret );

    // Store all metadata as Node Desscription strings
    void store( osg::Node* node );

    static const std::string LODCenterName;
    static const std::string LODInName;
    static const std::string LODOutName;
    static const std::string SwitchNumMasksName;
    static const std::string SwitchCurrentMaskName;
    static const std::string SwitchMaskNName;


protected:
    typedef std::map< std::string, std::string > MetaDataMap;
    MetaDataMap _map;

    const Nd_Token _handleType;
    char* _handleName;
};


#endif
