#include <set>
#include "../include/helper.hh"

int main()
{
    {// set
        print_header("Test set");
        std::set<int> myset;
        for (int i=1; i<10; i++) myset.insert(i*10);
        INFO_STREAM("Remove unexist 110 from myset: "<<myset.erase(110));
    }
}