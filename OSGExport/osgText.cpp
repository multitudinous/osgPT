// Copyright 2011 Skew Matrix Software LLC. All rights reserved.

#include "main.h"

#include <osg/Geode>
#include <osgText/Text>

#include <string>


Nd_Void
osgProcessText( Nd_Walk_Tree_Info* Nv_Walk_Tree_Info_Ptr,
               char* Nv_Master_Object, Nd_Int Nv_Primitive_Number,
               char* instance_name, osg::Geode* geode )
{
    Nd_Token	primitive_type;
    wchar_t 	*Nv_Text_String;
    wchar_t 	*Nv_Font_Name;

    /* Email from R. Lansdale, 3/4/2011:
    Me: Of those parameters, which are useful?
    Lansdale: The only ones you ever need to look at are:
        Nt_WIDETEXT, (wchar_t *) &Nv_Text_String, Nt_CMDSEP,
        Nt_WIDEFONTNAME, (wchar_t *) &Nv_Font_Name, Nt_CMDSEP,
    */
    Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, Nv_Primitive_Number, &primitive_type,
        Nt_TEXT3D,
        Nt_WIDETEXT, (wchar_t *) &Nv_Text_String, Nt_CMDSEP,
        Nt_WIDEFONTNAME, (wchar_t *) &Nv_Font_Name,
        Nt_CMDEND);

    osg::ref_ptr< osgText::Text > newText = new osgText::Text;
    geode->addDrawable( newText.get() );

    const bool useVBO = ( export_options->osgUseBufferObjects != 0 );
    newText->setUseVertexBufferObjects( useVBO );
    newText->setUseDisplayList( !useVBO );
    newText->setColor( osg::Vec4( 1., 1., 1., 1. ) );

    osgText::String textString( Nv_Text_String );
    newText->setText( textString );

    std::string fontFile( "default" );
    osgText::String fontNameWide( Nv_Font_Name );
    std::string fontName( fontNameWide.createUTF8EncodedString() );
    if( fontName.find( "Times" ) != std::string::npos )
        fontFile = "times.ttf";
    else if( fontName.find( "Arial" ) != std::string::npos )
        fontFile = "Arial.ttf";
    newText->setFont( fontFile );


	// Compute character height from BB z extent.
	Nd_Vector Nv_Text_BBox_Min, Nv_Text_BBox_Max;
	Ni_Inquire_Object( Nv_Master_Object, Nt_BOUNDINGBOX, Nv_Text_BBox_Min, Nv_Text_BBox_Max, Nt_CMDEND );
	float height = Nv_Text_BBox_Max[2] - Nv_Text_BBox_Min[2];
	if( fabs( height ) < 1e-5 )
		height = 1e-5;
    newText->setCharacterSize( height );

    osgText::TextBase::AxisAlignment alignment = osgText::TextBase::XZ_PLANE;
    newText->setAxisAlignment( alignment );

    // Position LL corner at BB min value.
    newText->setPosition( osg::Vec3( Nv_Text_BBox_Min[0],
        Nv_Text_BBox_Min[1], Nv_Text_BBox_Min[2] ) );
    newText->setAlignment( osgText::TextBase::LEFT_BOTTOM );
}
