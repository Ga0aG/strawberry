#include <iostream>

#define DECLARE_ACTION(ACTION_NAME) \
    inline void Execute ## ACTION_NAME() { \
        std::cout << "TEST" << std::endl; \
    }

DECLARE_ACTION(Aaction); // Is should define outside main

int main()
{
    ExecuteAaction();
    return 0;
}