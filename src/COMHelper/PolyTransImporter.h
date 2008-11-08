#ifndef __POLYTRANSIMPORTER_H__
#define __POLYTRANSIMPORTER_H__

#include <string>


class PolyTransImporter
{

public:

	//CONSTRUCTOR
	PolyTransImporter( const std::string& aFileExtension, const std::string& anImporterName );


	//DESTRUCTOR
	~PolyTransImporter();


	//ACCESSORS
	const std::string& GetFileExtension() const;
	const std::string& GetImporterName() const;


private:

	//DATA MEMBERS
	std::string m_FileExtension;
	std::string m_ImporterName;
};


#endif // __POLYTRANSIMPORTER_H__
