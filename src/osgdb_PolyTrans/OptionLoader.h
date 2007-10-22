#ifndef __OPTION_LOADER_H__
#define __OPTION_LOADER_H__ 1

#include <string>
#include <map>


// This class is designed to parse a text istream consisting of comment lines
//   and parameter / value(s) lines.
//
// Leading blanks before the start of the parameter name are skipped. Any
//   number of spaces or tabs can separate the parameter from its value(s).
//   The parameter name can't contain whitespace.
//
// osgdb_PolyTrans uses semicolons to separate osgDB::ReaderWriter::Options
//   strings, which is passes to this class for parameter/value parsing.
//   Some osgdb_PolyTrans values use commas internally. However, this class
//   is ignorant of semicolons and commas.
//
// A parameter/value line has the following syntax:
//   [whitespace]parameter<whitespace>value[whitespace]
// For example:
//   CachedLoad true
//
// Recognized Booleans:
//   true: "true", "1", "on", "y", "yes"
//   false: "false", "0", "off", "n", "no"
//
// Comments are defined as lines starting with '/' or '#'.
class OptionLoader
{
public:
    OptionLoader();
    ~OptionLoader();

    // Load possibly multiple parameter/value(s) from the given istream.
    bool load( std::istream& in );

    bool getOption( const std::string& option, std::string& value ) const;
    bool getOption( const std::string& option, bool& value ) const;
    bool getOption( const std::string& option, int& value ) const;
    bool getOption( const std::string& option, float& value ) const;

private:
    typedef std::map<std::string,std::string> OptionMap;
    OptionMap _opts;

    static std::string trim( const std::string& str );
};


#endif
