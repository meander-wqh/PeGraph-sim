#ifndef OXT_H
#define OXT_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include "OXTEDB.h"
#include "../pbcwrapper/PBC.h"

using namespace std;

class OXT{
    public:
        OXT(const char* dataset);
        OXTEDB SetUp();
        G1 g;
        Pairing* p;
    private:
        std::ifstream file;
        int Fileline = 0; 
        unordered_map<string,vector<string>> DB;
};

#endif