// Copyright 2008-2009 Skew Matrix Software LLC. All rights reserved.

#include "main.h"
#include "osgOptimize.h"

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>
#include <osg/NodeVisitor>
#include <osg/MatrixTransform>
#include <sstream>

//#define POLYTRANS_OSG_EXPORTER_STRIP_ALL_NAMES // strips all human-recognizable names and replaces with generic name/number
//#define POLYTRANS_OSG_EXPORTER_FORCIBLY_OPTIMIZE // optimize everything heavily


// This NodeVisitor removes all node names 
class StripAllNames : public osg::NodeVisitor
{
public:
    StripAllNames( bool numberStrippedNames = true, osg::NodeVisitor::TraversalMode mode = osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN )
      : osg::NodeVisitor( mode ),
      _groupNameBase("Group"), _nodeNameBase("Node"), _geodeNameBase("Geode"), _lodNameBase("LOD"),
      _groupCount(0), _nodeCount(0), _geodeCount(0), _lodCount(0), _numberStrippedNames(numberStrippedNames)
    {}
    ~StripAllNames() {}

    void setGroupNameBase(const std::string newName) {_groupNameBase = newName;}
    void setNodeNameBase(const std::string newName) {_nodeNameBase = newName;}
    void setGeodeNameBase(const std::string newName) {_geodeNameBase = newName;}
    void setLODNameBase(const std::string newName) {_lodNameBase = newName;}

    void apply( osg::Node& node )
    {
        std::ostringstream formatter;
        formatter << _nodeNameBase;
        if(_numberStrippedNames)
        {
            formatter << " #" << _nodeCount++;
        } // if
        node.setName( formatter.str() );
        traverse( node );
    } // apply Node

    void apply( osg::Geode& node )
    {
        std::ostringstream formatter;
        formatter << _geodeNameBase;
        if(_numberStrippedNames)
        {
            formatter << " #" << _nodeCount++;
        } // if
        node.setName( formatter.str() );
        traverse( node );
    } // apply Geode

    void apply( osg::Group& node )
    {
        std::ostringstream formatter;
        formatter << _groupNameBase;
        if(_numberStrippedNames)
        {
            formatter << " #" << _nodeCount++;
        } // if
        node.setName( formatter.str() );
        traverse( node );
    } // apply Group

    void apply( osg::LOD& node )
    {
        std::ostringstream formatter;
        formatter << _lodNameBase;
        if(_numberStrippedNames)
        {
            formatter << " #" << _nodeCount++;
        } // if
        node.setName( formatter.str() );
        traverse( node );
    } // apply LOD

private:
    std::string _groupNameBase, _nodeNameBase, _geodeNameBase, _lodNameBase;
    unsigned _groupCount, _nodeCount, _geodeCount, _lodCount;
    bool _numberStrippedNames;
}; // StripAllNames



// This NodeVisitor removes names that were automatically generated by PolyTrans.
// This facilitates optimization; the osgUtil::Optimizer's REMOVE_REDUNDANT_NODES
// operation appears to flag as "not redundant" if the names differ.
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
        if( name.find( std::string( "body " ) ) == 0 )
            // Starts with "body "
            node.setName( "" );
        traverse( node );
    }
    void apply( osg::Geode& node )
    {
        std::string name( node.getName() );
        if( name.find( std::string( "face " ) ) == 0 )
            // Starts with "face "
            node.setName( "" );
        traverse( node );
    }
};


// Determine whether or not a Group 'grp' is redundant (and therefore can be removed).
// A Group is considered redundant if it meets the following criteria:
//  1. It must have only one child.
//  2. If it's a MatrixTransform, the matrix must be identity.
//  3. If the Group's name is not empty, then the child's name must either
//     be empty, or wholly contain the Group name.
//  4. If the Group's description list is not empty, then the child's description
//     list must either be empty or identical to the Group's description list.
bool
redundant( osg::Group* grp, osg::Node* child, std::string& preserveName, osg::Node::DescriptionList& preserveDL )
{
    bool redundant( true );

    if( grp->getNumChildren() > 1 )
    {
        // More than one child -- not redundant.
        redundant = false;
    }

    {
        osg::MatrixTransform* mt( dynamic_cast< osg::MatrixTransform* >( grp ) );
        if( mt != NULL )
        {
            if( !( mt->getMatrix().isIdentity() ) )
            {
                // MatrixTransform nodes w/ non-identity matrix are not redundant.
                redundant = false;
            }
        }
    }

    if( preserveName.empty() )
    {
        if( grp->getName().empty() )
            preserveName = child->getName();
        else
            preserveName = grp->getName();
    }
    if( !preserveName.empty() )
    {
        // If child name contains grp name, then the grp is redundant. Example:
        // grp name is "usmc23_4009.asm", child name is "usmc23_4009.asm #2".
        // grp is redundant in this case.
        // grp is also redundant if child name is empty.
        if( ( !child->getName().empty() ) &&
            ( child->getName().find( preserveName ) == child->getName().npos ) )
            // Child name does not contain parent name -- not redundant.
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

// Starting with the node passed in, walk down its children and
// 'remove' any Groups that can be identified as redundant.
// Return the first node we encounter that is not redundant.
// While walking down the scene graph, preserve the topmost
// non-empty node name and topmost non-empty description list.
osg::Node*
removeRedundantGroups( osg::Node* node )
{
    std::string preserveName( node->getName() );
    osg::Node::DescriptionList& preserveDL( node->getDescriptions() );
    osg::Node* localNode( node );
    osg::Group* grp( dynamic_cast< osg::Group* >( localNode ) );
    while( ( grp != NULL ) && ( grp->getNumChildren() > 0 ) )
    {
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
performSceneGraphOptimizations( osg::Node* model )
{
    osgUtil::Optimizer uOpt;

    osg::Node* newRoot( model );

    if (export_options->osgRunOptimizer)
    {
        // Remove some PolyTrans-inserted node names
        if( export_options->osgStripImplicitNames)
        {
            StripNames sgn;
            model->accept( sgn );
        }


        // Share StateSets and merge the Geometry objects.
        // Must use static object detection to detect static StateSets.
        unsigned int flags = osgUtil::Optimizer::STATIC_OBJECT_DETECTION;
        if( export_options->osgShareState )
            flags |= osgUtil::Optimizer::SHARE_DUPLICATE_STATE;
        if( export_options->osgMergeGeodes )
            flags |= osgUtil::Optimizer::MERGE_GEODES;

        if( flags != osgUtil::Optimizer::STATIC_OBJECT_DETECTION )
            uOpt.optimize( model, flags );


        // Merge Geometry and remove any redundant nodes.
        uOpt.reset();
        flags = osgUtil::Optimizer::STATIC_OBJECT_DETECTION;
        if( export_options->osgMergeGeometry )
            flags |= osgUtil::Optimizer::MERGE_GEOMETRY;
        if( export_options->osgRemoveRedundantNodes )
            flags |= osgUtil::Optimizer::REMOVE_REDUNDANT_NODES;

        if( flags != osgUtil::Optimizer::STATIC_OBJECT_DETECTION )
            uOpt.optimize( model, flags );


        // Remove groups with one child, preserving topmost
        // node names and description lists.
        if( export_options->osgRemoveRedundantNodes )
            newRoot = removeRedundantGroups( model );
        else
            newRoot = model;


        // Tristrip geometry
        uOpt.reset();
        flags = 0;
        if( export_options->osgCreateTriangleStrips)
            flags |= osgUtil::Optimizer::TRISTRIP_GEOMETRY;

        if( flags != 0 )
            uOpt.optimize( newRoot, flags );
    }

// this operation is done after optimization, so as not to interfere with
// the legitimate StripNames functionality which could break due to every
// node acquiring a unique name during StripAllNames
#ifdef POLYTRANS_OSG_EXPORTER_STRIP_ALL_NAMES
    // this is set at compile time, not as a runtime option
    StripAllNames stripAllNames(false);
    model->accept( stripAllNames );
#endif // POLYTRANS_OSG_EXPORTER_STRIP_ALL_NAMES

// do another round of forcible optimizationg to lean down the structure
// as much as possible
#ifdef POLYTRANS_OSG_EXPORTER_FORCIBLY_OPTIMIZE
    osgUtil::Optimizer fOpt;
    unsigned int flags = osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS;
    uOpt.optimize( newRoot, flags );
    // now spatialize any remaining groups
    flags = osgUtil::Optimizer::SPATIALIZE_GROUPS;
    uOpt.optimize( newRoot, flags );
#endif // POLYTRANS_OSG_EXPORTER_FORCIBLY_OPTIMIZE


    return( newRoot );
}
