
#ifndef __OSG_INSTANCE_H__
#define __OSG_INSTANCE_H__


#include "main.h"
#include <osgDB/ReaderWriter>
#include <string>

namespace osg {
    class Node;
}


osg::Node* findNodeForInstance( const std::string& name );
osg::Node* createReferenceToInstance( const std::string& name );
bool addInstance( const std::string& name, osg::Node* root );
unsigned int getNumberOfInstances();
void writeInstancesAsFiles( const std::string& extension, osgDB::ReaderWriter::Options* opt );
void clearInstances();


#endif
