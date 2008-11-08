
#ifndef __OSG_INSTANCE_H__
#define __OSG_INSTANCE_H__


#include "main.h"
#include <osgDB/ReaderWriter>
#include <string>

class osg::Node;


struct InstanceInfo
{
    InstanceInfo()
      : _refCount( 0 )
    {}
    ~InstanceInfo()
    {}

    osg::ref_ptr< osg::Node > _subgraph;
    std::string _fileName;
    unsigned int _refCount;
};


bool doesInstanceExist( const std::string& key );
InstanceInfo* getInstance( const std::string& key );
void addInstance( const std::string& key, const InstanceInfo& iInfo );

unsigned int getNumberOfInstances();
void writeInstancesAsFiles( const std::string& extension, const osgDB::ReaderWriter::Options* opt );
void clearInstances();


#endif
