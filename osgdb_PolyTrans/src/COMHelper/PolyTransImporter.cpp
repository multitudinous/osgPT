#include "COMHelper/PolyTransImporter.h"



PolyTransImporter::PolyTransImporter( const std::string& aFileExtension, const std::string& anImporterName ) :
	m_FileExtension( aFileExtension ),
	m_ImporterName( anImporterName )
{
}


PolyTransImporter::~PolyTransImporter()
{
}


const std::string&
PolyTransImporter::GetFileExtension() const
{
	return m_FileExtension;
}


const std::string&
PolyTransImporter::GetImporterName() const
{
	return m_ImporterName;
}