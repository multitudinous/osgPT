
#ifndef __OSG_SURFACE_H__
#define __OSG_SURFACE_H__


#include "main.h"
#include <string>

namespace osg {
    class Material;
    class Texture2D;
}


Nd_Int createMaterialCB( Nd_Enumerate_Callback_Info *cbi_ptr );
Nd_Int createTextureCB( Nd_Enumerate_Callback_Info *cbi_ptr );

void lookupSurface( const std::string& name, osg::Material** mat, osg::Texture2D** tex );


#endif
