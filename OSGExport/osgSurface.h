
#ifndef __OSG_SURFACE_H__
#define __OSG_SURFACE_H__


#include "main.h"
#include <osg/ref_ptr>
#include <osg/Material>
#include <string>

namespace osg {
    class Material;
    class Texture2D;
}


struct SurfaceInfo
{
    SurfaceInfo()
        : _faceAlpha( 1. ), _reflectAlpha( 1. )
    {}
    SurfaceInfo( const SurfaceInfo& si )
    {
        _mat = si._mat.get();
        _tex = si._tex.get();
        _faceAlpha = si._faceAlpha;
        _reflectAlpha = si._reflectAlpha;
    }
    ~SurfaceInfo()
    {}

    osg::ref_ptr< osg::Material > _mat;
    osg::ref_ptr< osg::Texture2D > _tex;

    float _faceAlpha;
    float _reflectAlpha;
};


Nd_Int createMaterialCB( Nd_Enumerate_Callback_Info *cbi_ptr );
Nd_Int createTextureCB( Nd_Enumerate_Callback_Info *cbi_ptr );

const SurfaceInfo& lookupSurface( const std::string& name );


#endif
