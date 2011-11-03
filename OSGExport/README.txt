To control if meta data is stored uncomment this:

#define POLYTRANS_OSG_EXPORTER_STRIP_METADATA

in osgMetaData.cpp

To control if debug info is used for multiple textures uncomment this:

#define POLYTRANS_OSG_EXPORTER_ANNOTATE_TEXTURE_USAGE_DEBUG

in osgSurface.cpp

To control if groups are optimized out and other file specific data is optimized out uncomment this:

#define POLYTRANS_OSG_EXPORTER_STRIP_ALL_NAMES // strips all human-recognizable names and replaces with generic name/number
#define POLYTRANS_OSG_EXPORTER_FORCIBLY_OPTIMIZE // optimize everything heavily

in osgOptimize.cpp

