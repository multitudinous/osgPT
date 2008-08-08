
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


#define	IGNORE_RED_FOLDERS_IN_HIERARCHY	Nc_TRUE

Nd_Bool osgProcessMesh( Nd_Walk_Tree_Info *Nv_Info, char *master_object, osg::Geode* geode );

extern Nd_Void	NI_Exporter_DAGPath_UserDataMemoryFreeRoutine(void *data);


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
	// In case there are 'red folders' (empty objects) in the hierarchy
	// tree (cases of object instancing) we have to keep track of each
	// instance in the tree based on DAG Paths (see explanation in the 
	// file ni4_aux.h). This function adds a new DAG Path to this current
	// empty instance. To uniquely find the parent of this instance in the 
	// the tree call the 'Ni_FindParentViaDAGPathLineage()' function (see above)
	//
	// NOTE: An exporter can associate its own local data with this newly
	//       allocated DAG Path node by supplying its pointer to the 
	//	'Nv_DAGPath_User_Data_Pointer' argument. If you allocate memory and pass in this memory pointer
	// 	then you will have to specify the 'NI_Exporter_DAGPath_UserDataMemoryFreeRoutine'
	//	argument which is the callback function called by the toolkit
	//	to free up this memory at the end of the export process, otherwise
	//	set the function pointer argument to NULL and no callbacks will be
	// 	called by the toolkit.
	Nd_Void			*Nv_DAGPath_User_Data_Pointer( NULL );
	Nd_HierarchyDAGPath_Info *hierarchy_node =
        Ni_AddHierarchyDAGPath(Nv_Info, NI_Exporter_DAGPath_UserDataMemoryFreeRoutine, (void *) Nv_DAGPath_User_Data_Pointer);

	// If we don't care about "red folders" in the hierarchy then let's
	// return now. The red folder will be added to the DAG Path lists
	// above (via 'Ni_AddHierarchyDAGPath()') and the 'Nv_IgnoreEmptyObjectNodesInTree'
	// option above will effectively make them invisible to this exporter.
	// In general you would only want to deal with red folders if your
	// exporter allows sub-sections of a hierarchy tree to be instantiated
	// in some manner; normally no file formats allow this to happen.
#if IGNORE_RED_FOLDERS_IN_HIERARCHY
	if (Nv_Info->Nv_Empty_Object)
		return;
#endif

    // Get the instance's handleName
	char* handleName = Nv_Info->Nv_Handle_Name;
    std::string extension( (char *)( Nv_Info->Nv_User_Data_Ptr1 ) );

    // If this is a shared instance, "reference" will be valid.
    osg::ref_ptr< osg::Node > reference( NULL );

    // If this is an instance, masterName will be the instance name.
    std::string masterName;
    bool processSubgraph( true );

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
            // Ni_Inquire_Object options:
            // Nt_NUMINSTANCES, (Nd_Int *)&num_derived_instances, Nt_CMDSEP,
            // Nt_NUMCHILDREN, (Nd_Int *)&num_children_instances, Nt_CMDSEP,

            masterName = std::string( masterObject );
            if (!masterName.empty())
            {
                // This is an instance of a master object.

                if ( findNodeForInstance( masterName ) )
                {
                    // We've already processed the master object and it's stored
                    //   as a subtree in the instance map. Get a reference to it.
                    reference = createReferenceToInstance( masterName, handleName, extension );
                    processSubgraph = false;
                }

                else if (export_options->osgInstanceFile)
                {
                    // We haven't seen it before, so get the reference to the
                    // (non-existing) file and process the subgraph anyway.
                    reference = createReferenceToInstance( masterName, handleName, extension );
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


    // Creating a Group node here. This facilitates instancing, and
    // allows us to attach metadata to the Group rather than the instance.
    // If we attach metadata to the instance, then it would end up
    // being attached to all references to the instance.
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
    // Nodes with Geometry can also have children; our new Group
    // will be the parent pushed onto the parentStack.
    _lastNode = newGroup.get();

    if (!processSubgraph)
    {
        newGroup->addChild( reference.get() );
        return;
    }
    if (reference.valid())
    {
        // We're writing instances as files and this is the first time
        // we've seen this instance. So, we stick in the ProxyNode
        // as a child, but we continue to process the subgraph.
        newGroup->addChild( reference.get() );
        newGroup = NULL;
    }


    osg::ref_ptr< osg::Geode > geode = new osg::Geode;
    geode->setName( handleName );
    if (newGroup.valid())
        newGroup->addChild( geode.get() );

    // If this is an instance of a master object, add it.
    if (!masterName.empty())
    {
        addInstance( masterName, geode.get() );
		++export_options->total_instances;
    }

    // Add mesh data to this Geode.
    if (osgProcessMesh( Nv_Info, masterObject, geode.get() ))
        Ni_Report_Error_printf( Nc_ERR_WARNING, "walkTreeCallback: Error return from osgProcessMesh.\n" );
}

static Nd_Void
changeLevelCallback(Nd_Token Nv_Parent_Type, char *Nv_Parent_Handle_Name, 
	Nd_Short Nv_Current_Hierarchy_Level, Nd_Short Nv_Starting_New_Level)
{
#if IGNORE_RED_FOLDERS_IN_HIERARCHY
	if (Nv_Parent_Type == Nt_OBJECT)
		return;
#endif

    if (Nv_Starting_New_Level != 0)
        _parentStack.push_back( _lastNode->asGroup() );
    else
        _parentStack.pop_back();
}

void
writeOSG( const char* out_filename, long *return_result )
{
	// Init the DAG Path hierarchy lists (used to uniquely identify any 
	// instance or empty object (red folder) in the hierarchy tree)
#if IGNORE_RED_FOLDERS_IN_HIERARCHY
	Nd_Int		Nv_IgnoreEmptyObjectNodesInTree = Nc_TRUE;
#else
	Nd_Int		Nv_IgnoreEmptyObjectNodesInTree = Nc_FALSE;
#endif
	Ni_InitHierarchyDAGPathLists(Nv_IgnoreEmptyObjectNodesInTree);

    Nd_Int dummy;

    std::string fileName( out_filename );
    std::string extension = osgDB::getFileExtension( fileName );

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

		Ni_Walk_Tree(NULL, (Nd_Ptr)( extension.c_str() ), (Nd_Void *) NULL, walkTreeCallback,
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

    // Set OSG .ive/.osg export plugin Options.
    osgDB::ReaderWriter::Options* opt = new osgDB::ReaderWriter::Options;
    const bool isIVE = osgDB::equalCaseInsensitive( extension, "ive" );
    if (isIVE)
        // Writing a .ive file. Tell it we don't have any texture image data.
        // When the .ive gets loaded, this forces the plugin to read the texture image file.
        opt->setOptionString( "noTexturesInIVEFile" );

    // The grand finale: Write the scene graph as a file.
    bool success = osgDB::writeNodeFile( *_root, fileName, opt );
    if (!success)
    {
        Ni_Report_Error_printf(Nc_ERR_FATAL, "writeOSG: writeNodeFile failed.");
        *return_result = Nc_TRUE;
        return;
    }

    // Now that we've written the file, also write the instances
    //   (if requested to do so) and clear the instance map.
    if (export_options->osgInstanceFile)
        writeInstancesAsFiles( extension, opt );
    clearInstances();
}
