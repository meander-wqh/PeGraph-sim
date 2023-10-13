#include "OXTProtocol.h"
#include <iostream>
using namespace std;

int main(){
    cout<<"test"<<endl;
    OXT oxt("dataset/com-orkut.ungraph-1kw.txt");
    OXTEDB oxtedb = oxt.SetUp();
    return 0;
}