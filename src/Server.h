#ifndef SINOPARSER_SERVER_H
#define SINOPARSER_SERVER_H

#include "Parser.h"

#include <iostream>

namespace MeCab {
    class Tagger;
}


class Server {
    public:
        Parser parser;
        Server(std::string address, int port);
};


#endif

