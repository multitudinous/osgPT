#ifndef OSGOQ_POLYTRANSIMPORTER_H
#define OSGOQ_POLYTRANSIMPORTER_H

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


#endif //OSGOQ_POLYTRANSIMPORTER_H