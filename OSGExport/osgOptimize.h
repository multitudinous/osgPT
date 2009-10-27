// Copyright 2008-2009 Skew Matrix Software LLC. All rights reserved.

#ifndef __OSG_OPTIMIZE_H__
#define __OSG_OPTIMIZE_H__ 1

#include <osg/Node>

namespace osg {
    class Node;
}

osg::Node* performSceneGraphOptimizations( osg::Node* model );


// __OSG_OPTIMIZE_H__
#endif
