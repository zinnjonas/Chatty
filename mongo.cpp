#include "mongo.h"

#include <iostream>

Mongo* Mongo::m_instance = nullptr;


Mongo::Mongo()
{
    Status status = client::initialize();
    
    if( status.isOK() )
        m_init = true;
    else
        m_init = false;
    
}

Mongo::~Mongo()
{
}

Mongo* Mongo::get_instance()
{
    if( m_instance == nullptr)
        m_instance = new Mongo();
    if( m_instance->is_ok())
        return m_instance;
    return nullptr;
}

void Mongo::release()
{
    if( m_instance)
        delete m_instance;
}

bool Mongo::is_ok()
{
    return m_init;
}

bool Mongo::connect( string db)
{
    try
    {
        m_conn.connect(db.c_str());
    }
    catch(const DBException &e)
    {
        cerr << "caught " << e.what() << endl;
        return false;
    }
    return true;
}

void Mongo::select_db(string db)
{
    m_db = db;
}

void Mongo::select_collection(string col)
{
    m_col = col;
}

bool Mongo::write(BSONObj obj, string collection, string db)
{
    if( m_instance && m_init)
    {
        string tmp;
        if( db.empty())
            tmp = m_db;
        else
            tmp = db;
        tmp += ".";
        if( collection.empty())
            tmp += m_col;
        else
            tmp += collection;
        m_conn.insert(tmp.c_str(), obj);
    
        string err = m_conn.getLastError();
    
        if( err.empty())
            return true;
        
    cerr << err << endl;
    }
    return false;
}

void Mongo::read( vector<BSONObj>& results, BSONObj obj, string collection, string db)
{
    
    string tmp;
    if( db.empty())
        tmp = m_db;
    else
        tmp = db;
    tmp += ".";
    if( collection.empty())
        tmp += m_col;
    else
        tmp += collection;
    unique_ptr<DBClientCursor> up = m_conn.query(tmp.c_str(), obj);
    while( up->more())
        results.push_back(up->next());
}

BSONObj Mongo::read_one(BSONObj obj, string collection, string db)
{
    string tmp;
    if( db.empty())
        tmp = m_db;
    else
        tmp = db;
    tmp += ".";
    if( collection.empty())
        tmp += m_col;
    else
        tmp += collection;
    unique_ptr<DBClientCursor> up = m_conn.query(tmp.c_str(), obj);
    while( up->more())
        return up->next();
    return BSONObj();
}

size_t Mongo::count(string collection, string db)
{
    if( m_instance && m_init)
    {
        string tmp;
        if( db.empty())
            tmp = m_db;
        else
            tmp = db;
        tmp += ".";
        if( collection.empty())
            tmp += m_col;
        else
            tmp += collection;
    
        return m_conn.count(tmp.c_str());
        
    }
    return -1;
}

void Mongo::delete_all(string collection, string db)
{
    if( m_instance && m_init)
    {
        string tmp;
        if( db.empty())
            tmp = m_db;
        else
            tmp = db;
        tmp += ".";
        if( collection.empty())
            tmp += m_col;
        else
            tmp += collection;
            
        m_conn.remove(tmp, BSONObj());
    }
}
