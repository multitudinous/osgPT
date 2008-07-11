
#include "main.h"
#include "osg.h"
#include "osgSurface.h"
#include "osgMetaData.h"
#include "osgInstance.h"

#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/LOD>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/CullFace>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgUtil/Optimizer>
#include <string>
#include <deque>
#include <map>


Nd_Bool osgProcessMesh( Nd_Walk_Tree_Info *Nv_Info, char *master_object, osg::Geode* geode );


osg::ref_ptr< osg::Group > _root;
osg::ref_ptr< osg::Group > _parent;
osg::ref_ptr< osg::Node > _lastNode;

typedef std::deque< osg::Group* > GroupStack;
GroupStack _parentStack;

osg::Group*
getCurrentParent()
{
    if (_parentStack.empty())
        return NULL;
    else
        return _parentStack.back();
}


osg::Matrix
convertMatrix( Nd_Matrix in )
{
    osg::Matrix out;
    int iIdx, jIdx;
    for (iIdx=0; iIdx<4; iIdx++)
        for (jIdx=0; jIdx<4; jIdx++)
            out( iIdx, jIdx ) = in[ iIdx ][ jIdx ];

    return out;
}


static Nd_Void
walkTreeCallback(Nd_Walk_Tree_Info *Nv_Info, Nd_Int *Nv_Status)
{
    // Get the instance's handleName
	char* handleName = Nv_Info->Nv_Handle_Name;

    // If this is a shared instance, "reference" will be valid.
    osg::ref_ptr< osg::Node > reference( NULL );

    // If this is an instance, masterName will be the instance name.
    std::string masterName;

    char* masterObject( NULL );
    if (!export_options->osgInstanceIgnore)
    {
        // We're not ignoring instances. We need to handle them
        //   in some way....
        //
        //  * ALWAYS process the first occurance and store in the
        //    instance map in osgInstance.cpp.
        //  * If writing as files:
        //    - Return a reference to an instance (this is
        //      something OSG can load offline, like a ProxyNode).
        //    - After we process the master file, write all
        //      instancves in the map as files.
        //  * If sharing instances:
        //    - Return the address of the instance subgraph.
        //
        // Many of the details are handled in osgInstance.cpp.

        // If this is not an empty instance ('yellow folder' or grouping instance node)...
	    if (!Nv_Info->Nv_Empty_Instance && !Nv_Info->Nv_Empty_Object)
        {
		    /* Get the master object from which this instance was derived */
		    Ni_Inquire_Instance( handleName,
			    Nt_MASTEROBJECT, (char **) &masterObject, Nt_CMDSEP,
			    Nt_CMDEND);

            masterName = std::string( masterObject );
            if (!masterName.empty())
            {
                // This is an instance of a master object.

                if ( findNodeForInstance( masterName ) )
                {
                    // We've already processed the master object and it's stored
                    //   as a subtree in the instance map. Get a reference to it.
                    reference = createReferenceToInstance( masterName );

                    // TBD attach it
                }
            }
        }
    }

    // Get matrix from this instance, if there is one.
    osg::Matrix m = convertMatrix( Nv_Info->Nv_CTM );
    const bool isMatrix( !m.isIdentity() );
    osg::ref_ptr< osg::MatrixTransform > mt;
    if (isMatrix)
        mt = new osg::MatrixTransform( m );

    // Collect all metadata and save as Node descriptions.
    MetaDataCollector mdc( Nt_INSTANCE, handleName );
    const bool isLOD( mdc.hasMetaData( MetaDataCollector::LODCenterName ) );
    const bool isSwitch( mdc.hasMetaData( MetaDataCollector::SwitchNumMasksName ) );

    /* If this is an empty instance (yellow folder) or empty object (red folder) then process them here as grouping nodes */
	if (Nv_Info->Nv_Empty_Instance || Nv_Info->Nv_Empty_Object || Nv_Info->Nv_Null_Object_To_Follow)
    {
        osg::ref_ptr< osg::Group > top, tail;
        top = new osg::Group;
        tail = top.get();

        if (isLOD)
        {
            osg::ref_ptr< osg::LOD > lod = new osg::LOD;
            mdc.configureLOD( lod.get() );
            lod->addChild( top.get() );
            top = lod->asGroup();
        }
        if (isMatrix)
        {
            mt->addChild( top.get() );
            top = mt->asGroup();
        }

        top->setName( handleName );
        mdc.store( top.get() );

        if (!_root.valid())
            _root = top.get();
        if (getCurrentParent() != NULL)
            getCurrentParent()->addChild( top.get() );

        _lastNode = tail.get();
        return;
    }


    // Must be a non-Group

	/* Determine how many primitives this object has (should only be 1 in all modern cases) */
    Nd_Int Nv_Num_Defined_Primitives;
	Ni_Inquire_Object(masterObject,
		Nt_NUMPRIMITIVES, (Nd_Int *) &Nv_Num_Defined_Primitives, Nt_CMDSEP,
		Nt_CMDEND);

    if (!Nv_Num_Defined_Primitives)
		return;


    // Use the reference to the instance, if we have one.
    // Otherwise, just create a new Group.
    osg::ref_ptr< osg::Group > newGroup = new osg::Group;
    newGroup->setName( handleName );
    mdc.store( newGroup.get() );

    if (isMatrix)
    {
        mt->addChild( newGroup.get() );
        if (!_root.valid())
            _root = mt.get();
        if (getCurrentParent() != NULL)
            getCurrentParent()->addChild( mt.get() );
    }
    else
    {
        if (!_root.valid())
            // A one-geode scene graph?
            Ni_Report_Error_printf( Nc_ERR_RAW_MSG, "walkTreeCallback: Adding Geode to uninitialized _root.\n" );
        if (getCurrentParent() != NULL)
            getCurrentParent()->addChild( newGroup.get() );
    }
    _lastNode = NULL;

    if (reference.valid())
    {
        newGroup->addChild( reference.get() );
        return;
    }


    osg::ref_ptr< osg::Geode > geode = new osg::Geode;
    newGroup->addChild( geode.get() );

    // If this is an instance of a master object, and it's the first
    //   instance, add it.
    if (!masterName.empty() && !reference.valid() )
    {
        addInstance( masterName, geode.get() );
		++export_options->total_instances;
    }

    // Add mesh data to this Geode.
    if (osgProcessMesh( Nv_Info, masterObject, geode.get() ))
        Ni_Report_Error_printf( Nc_ERR_RAW_MSG, "walkTreeCallback: Error return from osgProcessMesh.\n" );
}


static Nd_Void
changeLevelCallback(Nd_Token Nv_Parent_Type, char *Nv_Parent_Handle_Name, 
	Nd_Short Nv_Current_Hierarchy_Level, Nd_Short Nv_Starting_New_Level)
{
    if (Nv_Starting_New_Level != 0)
        _parentStack.push_back( _lastNode->asGroup() );
    else
        _parentStack.pop_back();
}

void
writeOSG( const char* out_filename, long *return_result )
{
	Nd_Int dummy;

    // Create map of texture definitions to osg::Texture objects.
	Ni_Enumerate( &dummy, "*", Nc_FALSE, (Nd_Void *) NULL, (Nd_Void *) 0,
		createTextureCB, Nt_TEXTURE, Nt_CMDEND);
    // Must iterate over textures before surfaces. Surface definitions
    //   reference texture definitions.

    // Create map of surface names to osg::Material objects.
    Ni_Enumerate( &dummy, "*", Nc_FALSE, (Nd_Void *) NULL, (Nd_Void *) 0,
        createMaterialCB, Nt_SURFACE, Nt_CMDEND);

    Nd_Int returnStatus;
	try {
        Nd_Token ena_instancing_detection_counts = (export_options->ena_instancing) ? Nt_ON : Nt_OFF;

		Ni_Walk_Tree(NULL, (Nd_Void *) NULL, (Nd_Void *) NULL, walkTreeCallback,
			Nt_RETURNSTATUS, (Nd_Int *) &returnStatus, Nt_CMDSEP,
			// Make sure some internal state is set up for data export queries
			Nt_SETUPFOREXPORTER, Nt_ENABLED, Nt_ON, Nt_CMDSEP,
			// Specify a callback which will inform us when a new hierarchy level is started/ended
			Nt_STARTENDHIERARCHYCB, (void *) changeLevelCallback, Nt_CMDSEP,
			// This enables the internal pre-walk routines which determine the state and counts for the instancing
			// helper detection variables found at the end of the Nd_Walk_Tree data structure (see ni.h). See above
			// for a further explanation, or the Nd_Walk_Tree data structure for full variables explanations.
			Nt_ENABLEINSTANCINGDETECTIONCOUNTS, Nt_ENABLED, ena_instancing_detection_counts, Nt_CMDSEP,
			Nt_CMDEND);
		}
	catch( ... )
	{
		Ni_Report_Error_printf(Nc_ERR_ERROR, "OSG exporter raised an unknown exception while walking the geometry tree.");
		*return_result = Nc_TRUE;
        return;
	}

    // Matrices could scale normals, so normalize.
    if (export_options->osgNormalization)
        _root->getOrCreateStateSet()->setMode( GL_NORMALIZE, osg::StateAttribute::ON );

    if (export_options->osgBackfaceCulling)
    {
        osg::ref_ptr< osg::CullFace > cf = new osg::CullFace;
        _root->getOrCreateStateSet()->setAttributeAndModes( cf.get() );
    }

    // Possibly run the Optimizer
    if (export_options->osgRunOptimizer)
    {
        unsigned int flags( 0 );
        if (export_options->osgCreateTriangleStrips)
            flags |= osgUtil::Optimizer::TRISTRIP_GEOMETRY;
        if (export_options->osgMergeGeometry)
            flags |= osgUtil::Optimizer::MERGE_GEOMETRY;

        osgUtil::Optimizer opt;
        opt.optimize( _root.get(), flags );
    }

    std::string fileName( out_filename );
    std::string ext = osgDB::getFileExtension( fileName );
    const bool isIVE = osgDB::equalCaseInsensitive( ext, "ive" );

    // Set OSG .ive/.osg export plugin Options.
    osgDB::ReaderWriter::Options* opt = new osgDB::ReaderWriter::Options;
    if (isIVE)
        // Writing a .ive file. Tell it we don't have any texture image data.
        // When the .ive gets loaded, this forces the plugin to read the texture image file.
        opt->setOptionString( "noTexturesInIVEFile" );

    // The grand finale: Write the scene graph as a file.
    bool success = osgDB::writeNodeFile( *_root, fileName, opt );
    if (!success)
    {
        Ni_Report_Error_printf(Nc_ERR_ERROR, "OSG exporter: writeNodeFile failed.");
        *return_result = Nc_TRUE;
        return;
    }

    // Now that we've written the file, also write the instances
    //   (if requested to do so) and clear the instance map.
    if (export_options->osgInstanceFile)
        writeInstancesAsFiles( ext, opt );
    clearInstances();
}
