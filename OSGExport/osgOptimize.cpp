// Copyright 2008-2009 Skew Matrix Software LLC. All rights reserved.

#include "main.h"
#include "osgOptimize.h"

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>
#include <osg/NodeVisitor>



class StripNames : public osg::NodeVisitor
{
public:
    StripNames( osg::NodeVisitor::TraversalMode mode = osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN )
      : osg::NodeVisitor( mode )
    {}
    ~StripNames() {}

    void apply( osg::Group& node )
    {
        std::string name( node.getName() );
        if( name.find( std::string( "body " ) ) != name.npos )
            node.setName( "" );
        traverse( node );
    }
    void apply( osg::Geode& node )
    {
        node.setName( "" );
        traverse( node );
    }
};


bool
redundant( osg::Group* grp, osg::Node* child, std::string& preserveName, osg::Node::DescriptionList& preserveDL )
{
    bool redundant( true );

    if( preserveName.empty() )
    {
        if( grp->getName().empty() )
            preserveName = child->getName();
        else
            preserveName = grp->getName();
    }
    if( !preserveName.empty() )
    {
        if( ( !child->getName().empty() ) &&
            ( child->getName().find( preserveName ) == child->getName().npos ) )
            // Names are different -- not redundant.
            redundant = false;
    }

    if( preserveDL.empty() )
    {
        if( grp->getDescriptions().empty() )
            preserveDL = child->getDescriptions();
        else
            preserveDL = grp->getDescriptions();
    }
    if( !preserveDL.empty() )
    {
        if( ( !child->getDescriptions().empty() ) &&
            ( preserveDL != child->getDescriptions() ) )
            // Description lists are different -- not redundant.
            redundant = false;
    }

    return( redundant );
}

osg::Node*
removeRedundantGroups( osg::Node* node )
{
    std::string preserveName( node->getName() );
    osg::Node::DescriptionList& preserveDL( node->getDescriptions() );
    osg::Node* localNode( node );
    osg::Group* grp( dynamic_cast< osg::Group* >( localNode ) );
    while( grp != NULL )
    {
        if( grp->getNumChildren() > 1 )
        {
            grp->setName( preserveName );
            grp->setDescriptions( preserveDL );
            return( localNode );
        }

        osg::Node* child = grp->getChild( 0 );

        if( redundant( grp, child, preserveName, preserveDL ) )
        {
            localNode = child;
            grp = dynamic_cast< osg::Group* >( localNode );
        }
        else
        {
            grp->setName( preserveName );
            grp->setDescriptions( preserveDL );
            return( localNode );
        }
    }

    localNode->setName( preserveName );
    localNode->setDescriptions( preserveDL );
    return( localNode );
}


osg::Node*
performSceneGraphOptimizations( osg::Node* model, unsigned int flags )
{
    osgUtil::Optimizer uOpt;

    // Share StateSets and merge the Geometry objects.
    // Must use statis object detection to detect static StateSets.
    uOpt.optimize( model,
        osgUtil::Optimizer::STATIC_OBJECT_DETECTION |
        osgUtil::Optimizer::SHARE_DUPLICATE_STATE |
        osgUtil::Optimizer::MERGE_GEODES );

    // Remove any redundant nodes.
    uOpt.reset();
    uOpt.optimize( model,
        osgUtil::Optimizer::STATIC_OBJECT_DETECTION |
        osgUtil::Optimizer::MERGE_GEOMETRY |
        osgUtil::Optimizer::REMOVE_REDUNDANT_NODES );

    // Remove some PolyTrans-inserted nodes names
    StripNames sgn;
    model->accept( sgn );

    // Remove groups with one child, preserving topmost
    // node names and description lists.
    osg::Node* newRoot = removeRedundantGroups( model );

    return( newRoot );
}
