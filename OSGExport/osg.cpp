#include <windows.h>
#include "main.h"
#include "osg.h"
#include "osgSurface.h"
#include "osgMetaData.h"
#include "osgInstance.h"
#include "osgOptimize.h"

#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/ProxyNode>
#include <osg/LOD>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/CullFace>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <string>
#include <deque>
#include <map>


#define	IGNORE_RED_FOLDERS_IN_HIERARCHY	Nc_TRUE
//#define POLYTRANS_OSG_EXPORTER_STATIC_GROUPS // make all groups have STATIC DataVariance to facilitate forced optimization

Nd_Bool osgProcessMesh( Nd_Walk_Tree_Info *Nv_Info, char *master_object, osg::Geode* geode );
Nd_Void osgProcessText( Nd_Walk_Tree_Info* Nv_Walk_Tree_Info_Ptr,
	char* Nv_Master_Object, Nd_Int Nv_Primitive_Number,
    char* instance_name, osg::Geode* geode );

extern Nd_Void	NI_Exporter_DAGPath_UserDataMemoryFreeRoutine(void *data);


osg::ref_ptr< osg::Group > _root;
osg::ref_ptr< osg::Group > _parent;
osg::ref_ptr< osg::Node > _lastNode;
unsigned int _numNodes;
unsigned int _nodeCount;

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


std::string
getKeyName( const std::string& name )
{
    return( name.substr( 0, name.find_last_of( " #" )-1 ) );
}

std::string
getParentName( Nd_Walk_Tree_Info* Nv_Info )
{
    if( Nv_Info->Nv_Hierarchy_Level < 2 )
        return( "" );

    Nd_Walk_Tree_Info** stack = Nv_Info->Nv_TreeInfoStackPtr;
    Nd_Walk_Tree_Info* parent = stack[ Nv_Info->Nv_Hierarchy_Level - 2 ];
    return( parent->Nv_Handle_Name );
}

bool
sharable( Nd_Walk_Tree_Info* Nv_Info, const std::string& handleNameStr, const std::string& keyName )
{
    if( export_options->osgInstanceIgnore )
        // Sharing is disabled by user.
        return false;

    if( !Nv_Info->Nv_Empty_Instance )
        // We only share yellow folders.
        return false;

    if( ( keyName == std::string( "face" ) ) ||
        ( keyName == std::string( "body" ) ) ||
        ( keyName == std::string( "world" ) ) )
        // Commonly used node name that we never want to treat as an instance
        return false;

    const std::string parentKeyName = getKeyName( getParentName( Nv_Info ) );
    if( keyName == parentKeyName )
        // Can't be an instance of our own parent
        return false;

    if( ( handleNameStr.find( "ASM" ) != handleNameStr.npos ) ||
        ( handleNameStr.find( "asm" ) != handleNameStr.npos ) ||
        ( handleNameStr.find( "PRT" ) != handleNameStr.npos ) ||
        ( handleNameStr.find( "prt" ) != handleNameStr.npos ) )
        // Must contain keyword "asm" or "prt", case insensitive.
        //   Note: This is ProE specific and might need to change in the future.
        // If we got this far and found the keyword, then this must be sharable.
        return true;
    else
        // Didn't find the keyword, not sharable.
        return false;
}

static Nd_Void
countNodesCallback(Nd_Walk_Tree_Info *Nv_Info, Nd_Int *Nv_Status)
{
#if IGNORE_RED_FOLDERS_IN_HIERARCHY
	if (Nv_Info->Nv_Empty_Object)
		return;
#endif
    _numNodes++;
}

static Nd_Void
walkTreeCallback(Nd_Walk_Tree_Info *Nv_Info, Nd_Int *Nv_Status)
{
	Nd_Token 		primitive_type;
	Nd_Int			Nv_Num_Defined_Primitives, jdx;
	char			*handle_name, *master_object;

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

    _nodeCount++;
    Export_IO_Check_For_User_Interrupt_With_Stats( _nodeCount, _numNodes );

	// If instance's "hidden" flag turned on, or inherited hidden,
    // then return.
    if( ( !export_options->osgWriteHiddenNodes ) &&
	    ( Nv_Info->Nv_Hidden_Flag || Nv_Info->Nv_Inherited_Hidden_Flag ) )
    {
		*Nv_Status = Nc_WALKTREE_IGNORE_SUBTREE;
		return;
	}

    // Get the instance's handleName
	char* handleName = Nv_Info->Nv_Handle_Name;
    std::string handleNameStr( handleName );
    std::string extension( (char *)( Nv_Info->Nv_User_Data_Ptr1 ) );

    Export_IO_UpdateStatusDisplay( "node", handleName, "Creating OSG scene graph data." );

    // For instancing, the key name is the handle name with "#<n>" stripped off the end.
    std::string keyName = getKeyName( handleNameStr );
    std::string instanceFileName = keyName + "." + extension;

    // Collect OSG instance information.
    bool sharableInstance( false );
    osg::ref_ptr< osg::Node > reference( NULL );
    bool processSubgraph( true );

    if( sharable( Nv_Info, handleNameStr, keyName ) )
    {
        sharableInstance = true;

        // This is a sharable instance. We need to handle them
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
        //  * If we're referencing an instance already in the instanceMap,
        //    increment its reference count.
        //
        // Many of the details are handled in osgInstance.cpp.

        const bool exists( doesInstanceExist( keyName ) );
        if( export_options->osgInstanceFile )
        {
            // Regardless of whether this instance already exists or not, create a ProxyNode
            //   reference to it.
            osg::ProxyNode* pn = new osg::ProxyNode;
            pn->setFileName( 0, instanceFileName );
            reference = pn;
            if( exists )
                // It's already in the map. No need to traverse this PolyTrans subgraph again.
                processSubgraph = false;
        }
        else if( exists )
        {
            // It's already in the map.
            processSubgraph = false;

            // Create a reference to the instance.
            InstanceInfo* iInfo = getInstance( keyName );
            iInfo->_refCount++;
            reference = iInfo->_subgraph.get();
        }
    }
    //Ni_Report_Error_printf( Nc_ERR_RAW_MSG, "Key: %s, Sharable: %d, exists: %d, reference: %x, pt ref %d.", keyName.c_str(), sharableInstance,
    //    doesInstanceExist( keyName ), reference.get(),
    //    Nv_Info->Nv_ObjectDefinition__InstanceReferenceCountInHierarchyTree );

    // Get matrix from this instance, if there is one.
    osg::Matrix m = convertMatrix( Nv_Info->Nv_CTM );
    const bool isMatrix( !m.isIdentity() );
    osg::ref_ptr< osg::MatrixTransform > mt;
    if (isMatrix)
    {
        mt = new osg::MatrixTransform( m );
        mt->setDataVariance( osg::Object::STATIC );
    }

    // Collect all metadata and save as Node descriptions.
    MetaDataCollector mdc( Nt_INSTANCE, handleName );
    const bool isLOD( mdc.hasMetaData( MetaDataCollector::LODCenterName ) );
    // Switch: Not currently used.
    const bool isSwitch( mdc.hasMetaData( MetaDataCollector::SwitchNumMasksName ) );


    osg::ref_ptr< osg::Node > newNode;
    {
        osg::Node* top( NULL );
        osg::Node* tail( NULL );

        if (isMatrix)
        {
            top = mt.get();
            tail = mt.get();
        }
        if (isLOD)
        {
            osg::LOD* lod = new osg::LOD;
            mdc.configureLOD( lod );
            if( tail != NULL )
            {
                // Add LOD as child of the MatrixTransform
                mt->addChild( lod );
            }
            else
            {
                // Just an LOD.
                top = lod;
            }
            tail = lod;
        }

        // So far, we have constructed a subgraph that is one of the following possibilities:
        // 1) Empty
        // 2) Just a MatrixTransform
        // 3) A MatrixTransform with an LOD child.

        // Next, add a new child to that subgraph. The child could be one of the following:
        // a) A reference to a sharable subgraph.
        // b) A Group node (if this PolyTrans node has no geometry).
        // c) A Geode (if this PolyTrans node does have geometry).
        if( reference.valid() )
        {
            newNode = reference;
        }
        else
        {
            if( Nv_Info->Nv_Empty_Instance || Nv_Info->Nv_Empty_Object || Nv_Info->Nv_Null_Object_To_Follow )
            {
                // Empty (no geometry) so create a Group to hold the children
                newNode = (osg::Node*) new osg::Group;
#ifdef POLYTRANS_OSG_EXPORTER_STATIC_GROUPS
				newNode->setDataVariance(osg::Object::STATIC);
#endif // POLYTRANS_OSG_EXPORTER_STATIC_GROUPS
            }
            else
                // Create a Geode to hold the geometry.
                newNode = (osg::Node*) new osg::Geode;
        }
        // Add the new node...
        if( tail != NULL )
        {
            osg::Group* grp = tail->asGroup();
            grp->addChild( newNode.get() );
            tail = newNode.get();
        }
        else
        {
            tail = newNode.get();
            top = tail;
        }

        // Record the node name on the tail node.
        if( sharableInstance )
        {
            // Sharable, so the node name will be the keyName (no " #<n>")
            if( !export_options->osgInstanceFile )
                // Do not assign the node name to tha tail node if we are writing files.
                //   "tail" is a ProxyNode in this case. The keyName is stored on the
                //   root of the shared subgraph, not on the ProxyNode.
                tail->setName( keyName );
        }
        else
            // Not sharing, just use the handleName.
            // TBD. Possibly change this? Might need to have it match the ProE part name...
            tail->setName( handleName );
        // Store metadata on the top node.
        mdc.store( top );

        // Add this new subgraph into the hierarchy.
        if (!_root.valid())
            _root = top->asGroup();
        if (getCurrentParent() != NULL)
            getCurrentParent()->addChild( top );

        _lastNode = tail;
    }

    // add the instance
    if( sharableInstance && processSubgraph)
    {
        if( export_options->osgInstanceFile )
        {
            // Ha! Overwrite _lastNode with a new Group. This will be
            //   the root of the shared subgraph.
            _lastNode = new osg::Group;
#ifdef POLYTRANS_OSG_EXPORTER_STATIC_GROUPS
			_lastNode->setDataVariance(osg::Object::STATIC);
#endif // POLYTRANS_OSG_EXPORTER_STATIC_GROUPS
            _lastNode->setName( keyName );
        }

        InstanceInfo iInfo;
        iInfo._subgraph = _lastNode.get();
        iInfo._fileName = instanceFileName;
        iInfo._refCount = 1;
        addInstance( keyName, iInfo );
    }

    // Support PolyTrans instancing -- This is differenct from OSG instancing.
    std::string masterName;
    char* masterObject( NULL );
    Nd_Int numPrimitives( 0 );
    // If this is not an empty instance ('yellow folder' or grouping instance node)...
    if (!Nv_Info->Nv_Empty_Instance && !Nv_Info->Nv_Empty_Object) {
        /* Get the master object from which this instance was derived */
        Ni_Inquire_Instance(handleName,
            Nt_MASTEROBJECT, (char **) &masterObject, Nt_CMDSEP,
            Nt_CMDEND);

        /* Determine how many primitives this object has (should only be 1 in all modern cases) */
        Ni_Inquire_Object(masterObject,
            Nt_NUMPRIMITIVES, (Nd_Int *) &numPrimitives, Nt_CMDSEP,
            Nt_CMDEND);
    }

    if (!processSubgraph)
    {
        // We're sharing and do not need to process the subgraph.
        //   Tell PolyTrans to ignore the subgraph and keep walking.
        *Nv_Status = Nc_WALKTREE_IGNORE_SUBTREE;
        return;
    }
    if (!numPrimitives)
        // No primitives to process. Continue walking the subgraph.
		return;


    // Add mesh data to this Geode.
    osg::Geode* geode = dynamic_cast< osg::Geode* >( newNode.get() );
    if( geode == NULL )
        Ni_Report_Error_printf( Nc_ERR_RAW_MSG, "Geode is NULL." );

	handle_name = Nv_Info->Nv_Handle_Name;
	if( !Nv_Info->Nv_Empty_Instance && !Nv_Info->Nv_Empty_Object )
    {
		/* Get the master object from which this instance was derived */
		Ni_Inquire_Instance(handle_name,
			Nt_MASTEROBJECT, (char **) &master_object, Nt_CMDSEP,
			Nt_CMDEND);

		/* Determine how many primitives this object has (should only be 1 in all modern cases) */
		Ni_Inquire_Object(master_object,
			Nt_NUMPRIMITIVES, (Nd_Int *) &Nv_Num_Defined_Primitives, Nt_CMDSEP,
			Nt_CMDEND);
	} else
    {
		Nv_Num_Defined_Primitives = 0;
		master_object = NULL;
	}

    for( jdx=0; jdx<Nv_Num_Defined_Primitives; ++jdx )
    {
		/* Get the type of primitive. Most will be indexed polygon meshes, NURBS surfaces, */
		/* NURBS curves or spline shapes. */
		Ni_Inquire_Primitive( Nt_OBJECT, master_object, jdx, (Nd_Token *) &primitive_type, Nt_CMDEND );

        if( !( export_options->osgTextPolygons ) &&
		    ( primitive_type == Nt_TEXT3D ) )
        {
            if( export_options->osgTextOSGText )
            {
                osgProcessText( Nv_Info, masterObject, jdx, handleName, geode );
            }
            else
            {
                // osgTextDiscard -- Do nothing.
            }
        }
        else if( osgProcessMesh( Nv_Info, masterObject, geode ) )
        {
            Ni_Report_Error_printf( Nc_ERR_WARNING, "walkTreeCallback: Error return from osgProcessMesh.\n" );
        }
    }
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
    osg::ref_ptr< osg::Node > writeCandidate;
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

        Export_IO_UpdateStatusDisplay( "", "", "Counting nodes." );
        _numNodes = 0;
        _nodeCount = 0;
		Ni_Walk_Tree(NULL, (Nd_Ptr)( extension.c_str() ), (Nd_Void *) NULL, countNodesCallback,
			Nt_RETURNSTATUS, (Nd_Int *) &returnStatus, Nt_CMDSEP,
			Nt_CMDEND);

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


    {
        // Possibly run the Optimizer
        writeCandidate = performSceneGraphOptimizations( _root.get() );

        // Set OSG .ive/.osg export plugin Options.
        osgDB::ReaderWriter::Options* opt = new osgDB::ReaderWriter::Options;
        const bool isIVE = osgDB::equalCaseInsensitive( extension, "ive" );
        if (isIVE)
            // Writing a .ive file. Tell it we don't have any texture image data.
            // When the .ive gets loaded, this forces the plugin to read the texture image file.
            opt->setOptionString( "noTexturesInIVEFile" );

        // The grand finale: Write the scene graph as a file.
        Export_IO_Check_For_User_Interrupt_With_Stats( 100, 100 );
        Export_IO_UpdateStatusDisplay( "file", (char *)(fileName.c_str()), "Exporting OSG file." );
        bool success = osgDB::writeNodeFile( *writeCandidate, fileName, opt );
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


    // Let go of ref_ptrs.
    _root = NULL;
    _parent = NULL;
    _lastNode = NULL;
}
