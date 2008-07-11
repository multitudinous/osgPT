
#include "main.h"
#include "osgInstance.h"

#include <osgDB/FileNameUtils>
#include <osgDB/WriteFile>
#include <osg/Node>
#include <osg/ref_ptr>
#include <string>
#include <map>


typedef std::map< std::string, osg::ref_ptr< osg::Node > > InstanceMap;
static InstanceMap _instanceMap;


osg::Node*
findNodeForInstance( const std::string& name )
{
    InstanceMap::iterator it = _instanceMap.find( name );
    if (it != _instanceMap.end())
        return( (it->second).get() );
    else
        return NULL;
}

osg::Node*
createReferenceToInstance( const std::string& name )
{
    osg::Node* root = findNodeForInstance( name );
    if (root == NULL)
    {
        Ni_Report_Error_printf( Nc_ERR_RAW_MSG, "createReferenceToInstance: can't find %s\n", name.c_str() );
        return NULL;
    }

    if (export_options->osgInstanceFile)
        return NULL;
    else if (export_options->osgInstanceShared)
        return( _instanceMap[ name ].get() );
    else
        return NULL;
}

bool
addInstance( const std::string& name, osg::Node* root )
{
    _instanceMap[ name ] = root;
    return true;
}

unsigned int
getNumberOfInstances()
{
    return( _instanceMap.size() );
}

void
writeInstancesAsFiles( const std::string& extension, osgDB::ReaderWriter::Options* opt )
{
    Ni_Report_Error_printf( Nc_ERR_RAW_MSG, "writeInstancesAsFiles: Not yet implemented.\n" );
}

void
clearInstances()
{
    _instanceMap.clear();
}
