#ifndef XAMPP_H
#define XAMPP_H

#include <string>
#include <vector>
#include <strophe.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "command.h"

using namespace std;

class Xmpp
{
    public:
        /**
         * @brief 
         */
        static Xmpp* get_instance();

        static void register_commands(Command* command);

        /**
         * @brief 
         */
        static void release();
              
        /**
         * @brief 
         * @param jid
         * @param pw
         */
        bool connect(string jid, string pw);
        /**
         * @brief 
         * @param conn
         * @param status
         * @param error
         * @param stream_error
         * @param userdata
         */
        static void connect_handler(xmpp_conn_t* const conn, const xmpp_conn_event_t status, const int error, xmpp_stream_error_t* const stream_error, void* const userdata);
        
        static int version_handler( xmpp_conn_t* const conn, xmpp_stanza_t* const stanza, void* const userdata);
        static int message_handler( xmpp_conn_t* const conn, xmpp_stanza_t* const stanza, void* const userdata);
        static int bob_handler( xmpp_conn_t* const conn, xmpp_stanza_t* const stanza, void* const userdata);
        
        static void parse_msg(xmpp_ctx_t *ctx, const char* const message, xmpp_stanza_t* msg);
        static void add_elements(xmpp_ctx_t *ctx, xmlNode* ele, xmpp_stanza_t* msg, string &plain_msg);
        
        static void send_image( xmpp_conn_t* const conn, string from, string file);
        
        void run();

    private:
        Xmpp();
        virtual ~Xmpp();
        
        static Xmpp* m_instance;
        
        static vector<string> m_bob;
        static string stuff;
        
        xmpp_ctx_t* m_ctx;
        xmpp_conn_t* m_conn;
        static Command* m_commands;
};



#endif // XAMPP_H
