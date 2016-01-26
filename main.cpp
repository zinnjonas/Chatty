#include <iostream>
#include <strophe.h>
#include <term.h>
#include <unistd.h>
#include <cryptopp/base64.h>

#include "mongo.h"
#include "xmpp.h"

using namespace std;
using namespace CryptoPP;

void end(int err)
{
    Mongo::get_instance()->release();
    Xmpp::release();
    if( err != 0)
        exit(err);
}

int main(int argc, char **argv)
{
    
    bool init = false;
    
    Mongo* mongo = Mongo::get_instance();
    if( mongo)
    {
        if( !mongo->connect())
        {
            cerr << "Unable to connect to database" << endl;
            end(1);
        }
        mongo->select_db("Chatty");
        mongo->select_collection("jabber");
        
        if( init)
        {
            mongo->delete_all();
        }
        
        // Test for the first run
        if( mongo->count() == 0)
        {
           string user;
           string domain;
           string resource;
           string pw;
           string tmp;
           
            cout << "Hello let me introduce myself. I am chatty and to be able to do what i am designed to i need a jabber account." << endl;
            cout << "So please give me a user" << endl;
            cin >> user;
            cout << "And the domain" << endl;
            cin >> domain;
            cout << "And a Resource" << endl;
            cin >> resource;
            cout << "Then I need the password for the " << user << "@" << domain << "/" << resource << " account" << endl;
            tmp = getpass("");
            new StringSource( tmp, true, new Base64Encoder(new StringSink(pw)));
            cout << "thx" << endl;
            mongo->write(BSON(GENOID << "user" << user.c_str() << "domain" << domain.c_str() << "resource" << resource.c_str() << "pw" << pw.c_str()));
            setupterm(nullptr, fileno(stdout), nullptr);
            putp(tigetstr("clear"));
        }
       
        // get the data from the database
        BSONObj jabberData = mongo->read_one();
        if( jabberData.isEmpty())
        {
            cerr << "no useful account could be found" << endl;
            end(1);
        }
        
        string password;
        new StringSource(jabberData["pw"].String(), true, new Base64Decoder(new StringSink(password)));
        string jabberid = jabberData["user"].str() + "@" + jabberData["domain"].str() + "/"+ jabberData["resource"].str();
        
        Xmpp* xampp = Xmpp::get_instance();
        
        if( !xampp->connect(jabberid, password))
        {
            cerr << "Error while connecting to jabber!" << endl;
            end(2);
        }
        
        xampp->run();
        
    }
    else
    {
        cerr << "No Database could be found!" << endl;
        end(1);
    }
    
    end(0);
    
    return 0;
}
