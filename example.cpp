#include <iostream>

using namespace std;

#include "argh.h"

int main(int argc, char* argv[])
{
    argh::parser cmdl;
    cmdl.parse(argc, argv, argh::PREFER_PARAM_FOR_UNREG_OPTION);

    if (cmdl["-v"])
        cout << "Verbose, I am." << endl;

    return EXIT_SUCCESS;
}
