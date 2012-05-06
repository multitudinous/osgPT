
#include "main.h"
#include "osgMetaData.h"

#include <osg/Node>
#include <osg/LOD>
#include <osg/Vec3>
#include <string>
#include <map>

//#define POLYTRANS_OSG_EXPORTER_STRIP_METADATA

const std::string MetaDataCollector::LODCenterName( "FLT-LOD-NODE-CENTER" );
const std::string MetaDataCollector::LODInName( "FLT-LOD-NODE-SWITCH-IN-DISTANCE" );
const std::string MetaDataCollector::LODOutName( "FLT-LOD-NODE-SWITCH-OUT-DISTANCE" );
const std::string MetaDataCollector::SwitchNumMasksName( "FLT-SWITCH-NODE-NUM-MASKS" );
const std::string MetaDataCollector::SwitchCurrentMaskName( "FLT-SWITCH-NODE-MASK1" );
const std::string MetaDataCollector::SwitchMaskNName( "FLT-SWITCH-NODE-MASK" );


static Nd_Int
collectMetaDataCB( Nd_Enumerate_Callback_Info *cbi_ptr )
{
    Nd_UShort    Nv_BDF_Save_Flag;
    Nd_Int        Nv_Num_Items;
    Nd_UShort    Nv_Data_Type;
    char         *Nv_Data_Ptr;

    if (!cbi_ptr->Nv_Matches_Made) 
        return(Nc_FALSE);

    Ni_User_Data_Format_DataElements_Into_Standardized_String2(
        (char*)cbi_ptr->Nv_Handle_Name1,         // This is the ID name of the user data item
        &Nv_BDF_Save_Flag, &Nv_Num_Items, &Nv_Data_Type, (Nd_Char **) &Nv_Data_Ptr,    // These are the returned meta data item contents
        cbi_ptr->Nv_Handle_Type2, (char*)cbi_ptr->Nv_Handle_Name2,     // This is the handle type and name, such as 'Nt_INSTANCE, "world"' 
        Nt_CMDEND );

    MetaDataCollector* mdc = (MetaDataCollector*) cbi_ptr->Nv_User_Data_Ptr1;
    mdc->add( std::string( cbi_ptr->Nv_Handle_Name1 ),
        std::string( Nv_Data_Ptr ) );

    return Nc_FALSE;    /* Do not terminate the enumeration */
}



MetaDataCollector::MetaDataCollector( Nd_Token Nv_Handle_Type, char *Nv_Handle_Name )
  : _handleType( Nv_Handle_Type ),
    _handleName( Nv_Handle_Name )
{
    Nd_Int    num_output = 0;

    Ni_Enumerate( &num_output, _handleName, Nc_FALSE,
        (Nd_Void *) this, (Nd_Void *) NULL,
        collectMetaDataCB,
        _handleType, Nt_USERDATA, Nt_CMDEND );
}

MetaDataCollector::~MetaDataCollector()
{
}

void
MetaDataCollector::add( std::string& name, std::string& value )
{
    _map[ name ] = value;
}

void
MetaDataCollector::configureLOD( osg::LOD* lod )
{
    double in, out;
    osg::Vec3 center;
    bool hasIn = getMetaData( LODInName, in );
    bool hasOut = getMetaData( LODOutName, out );
    bool hasCenter = getMetaData( LODCenterName, center );

    if (!hasIn || !hasOut || !hasCenter)
        return;
    lod->setCenter( center );
    lod->setRange( 0, out, in );
}

bool
MetaDataCollector::hasMetaData( const std::string& name )
{
    MetaDataMap::const_iterator it = _map.find( name );
    return( it != _map.end() );
}

bool
MetaDataCollector::getMetaData( const std::string& name, std::string& ret )
{
    MetaDataMap::const_iterator it = _map.find( name );
    bool found( it != _map.end() );
    if (found)
        ret = it->second;
    return found;
}

bool
MetaDataCollector::getMetaData( const std::string& name, double& ret )
{
    MetaDataMap::const_iterator it = _map.find( name );
    bool found( it != _map.end() );
    if (found)
    {
        Nd_UShort    Nv_BDF_Save_Flag;
        Nd_Int        Nv_Num_Items;
        Nd_UShort    Nv_Data_Type;
        Nd_Double   *Nv_Data_Ptr;

        Nd_UShort result = Ni_User_Data_Inquire2(
            const_cast< char* >( name.c_str() ),         // This is the ID name of the user data item
            &Nv_BDF_Save_Flag, &Nv_Num_Items, &Nv_Data_Type, (Nd_Void **) &Nv_Data_Ptr,    // These are the returned meta data item contents
            _handleType, _handleName,     // This is the handle type and name, such as 'Nt_INSTANCE, "world"' 
            Nt_CMDEND );
        if (result == Nc_FALSE)
            return false;

        ret = *( (double*)Nv_Data_Ptr );
    }
    return true;
}

bool
MetaDataCollector::getMetaData( const std::string& name, osg::Vec3& ret )
{
    MetaDataMap::const_iterator it = _map.find( name );
    bool found( it != _map.end() );
    if (found)
    {
        Nd_UShort    Nv_BDF_Save_Flag;
        Nd_Int        Nv_Num_Items;
        Nd_UShort    Nv_Data_Type;
        Nd_DblVector *Nv_Data_Ptr;

        Nd_UShort result = Ni_User_Data_Inquire2(
            const_cast< char* >( name.c_str() ),         // This is the ID name of the user data item
            &Nv_BDF_Save_Flag, &Nv_Num_Items, &Nv_Data_Type, (Nd_Void **) &Nv_Data_Ptr,    // These are the returned meta data item contents
            _handleType, _handleName,     // This is the handle type and name, such as 'Nt_INSTANCE, "world"' 
            Nt_CMDEND );
        if (result == Nc_FALSE)
            return false;

        ret[0] = ( (*Nv_Data_Ptr)[0] );
        ret[1] = ( (*Nv_Data_Ptr)[1] );
        ret[2] = ( (*Nv_Data_Ptr)[2] );
    }
    return true;
}

void
MetaDataCollector::store( osg::Node* node )
{
#ifndef POLYTRANS_OSG_EXPORTER_STRIP_METADATA
    MetaDataMap::const_iterator it = _map.begin();
    while (it != _map.end())
    {
        node->addDescription( it->first );
        node->addDescription( it->second );
        it++;
    }
#endif // !POLYTRANS_OSG_EXPORTER_STRIP_METADATA
}
