
#ifndef __OSG_SURFACE_H__
#define __OSG_SURFACE_H__


#include "main.h"
#include <osg/ref_ptr>
#include <osg/Material>
#include <osg/Texture2D>
#include <string>



struct SurfaceInfo
{
    SurfaceInfo()
        : _faceAlpha( 1. ), _reflectAlpha( 1. )
    {}
    SurfaceInfo( const SurfaceInfo& si )
    {
        _mat = si._mat.get();
        _tex = si._tex; // copy whole container
        _faceAlpha = si._faceAlpha;
        _reflectAlpha = si._reflectAlpha;
    }
    ~SurfaceInfo()
    {}

    osg::ref_ptr< osg::Material > _mat;
	std::vector< osg::ref_ptr< osg::Texture2D > > _tex; // vector to permit multiple texture layers per surface

    float _faceAlpha;
    float _reflectAlpha;
};


Nd_Int createMaterialCB( Nd_Enumerate_Callback_Info *cbi_ptr );
Nd_Int createTextureCB( Nd_Enumerate_Callback_Info *cbi_ptr );

const SurfaceInfo& lookupSurface( const std::string& name );


#endif
