#include <iostream>
#include "Server.h"
#include "Args.h"

int main(int argc, char* argv[]) {
    Args args(argc, argv);

    // otherwise std::cout << Glib::ustring doesn't work
    //
    setlocale(LC_ALL,"");

    Server server(args.listenHost ,args.listenPort );

    return 0;
}



