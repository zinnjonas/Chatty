#ifndef DATABASE_H
#define DATABASE_H

#include <string>

using namespace std;

class Database
{
    public:
        Database();
        virtual ~Database();

        virtual void release() = 0;
        
        virtual bool is_ok() = 0;
        virtual bool connect(string db) = 0;
    
};

#endif // DATABASE_H
