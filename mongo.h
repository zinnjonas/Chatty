#ifndef MONGO_H
#define MONGO_H

#include "database.h"

#include <mongo/client/dbclient.h>
#include <bson/bson.h>

#include <vector>

using namespace mongo;

class Mongo : public Database
{
    public:
       
        static Mongo* get_instance();
        void release();
        
        bool is_ok();
        
        virtual bool connect( string db = "localhost");
        
        void select_db( string db);
        void select_collection( string col);
        
        bool write( BSONObj obj, string collection = "", string db = "");
         void read( vector<BSONObj>& results, BSONObj obj = BSONObj(), string collection = "", string db = "");
         BSONObj read_one(BSONObj obj = BSONObj(), string collection = "", string db = "");
        
        size_t count(string collection = "", string db = "");
        
        void delete_all(string collection = "", string db = "");
        
        
    private:
        Mongo();
        ~Mongo();
        
        static Mongo* m_instance;
       
        bool m_init;
        
        DBClientConnection m_conn;
        string m_db;
        string m_col;
};

#endif // MONGO_H
