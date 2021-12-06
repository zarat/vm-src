#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <string>

class Parser {
 
    public:
        void parseFile(const std::string & in, const std::string & out);
        void setDebug(bool yes);
        
    private:
        bool debug = false;
};

#endif