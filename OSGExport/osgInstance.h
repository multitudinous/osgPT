
#ifndef __OSG_INSTANCE_H__
#define __OSG_INSTANCE_H__


#include "main.h"
#include <osgDB/ReaderWriter>
#include <string>

namespace osg {
    class Node;
}


osg::Node* findNodeForInstance( const std::string& name );
osg::Node* createReferenceToInstance( const std::string& objectName, const std::string& sourceFileName, const std::string& extension );
bool addInstance( const std::string& name, osg::Node* root );
unsigned int getNumberOfInstances();
void writeInstancesAsFiles( const std::string& extension, const osgDB::ReaderWriter::Options* opt );
void clearInstances();


#endif
