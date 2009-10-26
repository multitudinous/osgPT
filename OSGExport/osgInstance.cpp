
#include "main.h"
#include "osgInstance.h"

#include <osgDB/FileNameUtils>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osg/Node>
#include <osg/ProxyNode>
#include <osg/ref_ptr>
#include <string>
#include <map>


typedef std::map< std::string, InstanceInfo > InstanceMap;
static InstanceMap _instanceMap;


bool
doesInstanceExist( const std::string& key )
{
    return( _instanceMap.find( key ) != _instanceMap.end() );
}

InstanceInfo*
getInstance( const std::string& key )
{
    InstanceMap::iterator it = _instanceMap.find( key );
    if( doesInstanceExist( key ) )
        return &( _instanceMap[ key ] );
    else
        return NULL;
}

void
addInstance( const std::string& key, const InstanceInfo& iInfo )
{
    _instanceMap[ key ] = iInfo;
}


unsigned int
getNumberOfInstances()
{
    return( _instanceMap.size() );
}

void
writeInstancesAsFiles( const std::string& extension, const osgDB::ReaderWriter::Options* opt )
{
    // Possibly run the Optimizer
    unsigned int flags( 0 );
    if (export_options->osgRunOptimizer)
    {
        if (export_options->osgCreateTriangleStrips)
            flags |= osgUtil::Optimizer::TRISTRIP_GEOMETRY;
        if (export_options->osgMergeGeometry)
            flags |= osgUtil::Optimizer::MERGE_GEOMETRY;
    }

    int count( 0 );
    InstanceMap::iterator it = _instanceMap.begin();
    for( ; it != _instanceMap.end(); it++)
    {
        // Update progress bar and check for user abort.
        count++;
        if (Export_IO_Check_For_User_Interrupt_With_Stats( count, _instanceMap.size() ))
            return;

        InstanceInfo& iInfo = it->second;
        const std::string fileName = iInfo._fileName;
        osg::Node* node = iInfo._subgraph.get();

        osgUtil::Optimizer optimizer;
        optimizer.optimize( node, flags );

        Export_IO_UpdateStatusDisplay( "instance", (char *)(fileName.c_str()), "Exporting instance as file." );
        bool success = osgDB::writeNodeFile( *node, fileName, opt );
        if (!success)
            Ni_Report_Error_printf( Nc_ERR_INFO, "writeInstancesAsFiles: Error writing instance file \"%s\".", fileName.c_str() );
    }
}

void
clearInstances()
{
    _instanceMap.clear();
}
