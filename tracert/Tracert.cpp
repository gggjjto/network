#include <iostream>
#include <string>
#include "tracertRote.h"
int main()
{
    using namespace std;
    string s = "153.3.238.102"; 
    //cin>>tracertAdderss;
    //153.3.238.102
    //tracertRote::tracertRote(tracertAdderss);
    tracertRote ping(s);
    ping.tracertRotePing();

}
