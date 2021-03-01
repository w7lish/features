#include "Util.h"
#include <string.h>

void tokenize(const std::string& str, const std::string& s, std::vector<std::string>& tokens)
{
    // Use of strtok 
    char *tok = strtok(const_cast<char*>(str.c_str()), s.c_str()); 
  
    // Checks for delimeter 
    while (tok != 0) { 
        // printf(" %s\n", tok); 
        tokens.push_back(tok);
        tok = strtok(0, s.c_str()); 
    } 
}

void trim(std::string& str)
{
	size_t p = str.find_first_not_of( " \t" );
	str.erase( 0, p );

	p = str.find_last_not_of( " \t" );
	if( std::string::npos != p ) {
		str.erase( p + 1 );
	}
}
