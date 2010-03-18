
#include "main.h"
#include "osgInstance.h"
#include "osgOptimize.h"

#include <osgDB/FileNameUtils>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
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
        osg::ref_ptr< osg::Node > node = iInfo._subgraph.get();

        node = performSceneGraphOptimizations( node.get() );

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
