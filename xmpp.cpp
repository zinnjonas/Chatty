#include "xmpp.h"

#include <iostream>
#include <algorithm>
#include <cryptopp/files.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <chrono>
#include <thread>

#include "mongo.h"
#include "mime.h"

using namespace std;
using namespace CryptoPP;

Xmpp* Xmpp::m_instance = nullptr;
vector<string> Xmpp::m_bob;
string Xmpp::stuff;
Command* Xmpp::m_commands = nullptr;

Xmpp::Xmpp()
{
    // init the library
    xmpp_initialize();

    // create a new context
    m_ctx = xmpp_ctx_new(nullptr, nullptr);
    time_t t = time(nullptr);
    string dummy  = std::ctime(&t);
    SHA1 hash;
    new StringSource( dummy, true, new HashFilter( hash, new HexEncoder( new StringSink(stuff))));
    stuff = stuff.substr(stuff.length()-5);
}

Xmpp::~Xmpp()
{
    xmpp_conn_release(m_conn);
    xmpp_ctx_free(m_ctx);


    xmpp_shutdown();
}

Xmpp* Xmpp::get_instance()
{
    if (m_instance == 0) {
        m_instance = new Xmpp();
    }
    return m_instance;
}

void Xmpp::register_commands(Command* command)
{
  m_commands = command;
}

void Xmpp::release()
{
    if (m_instance) {
        delete m_instance;
    }
    m_instance = nullptr;
}

bool Xmpp::connect(string jid, string pw)
{
    // create a connection
    m_conn = xmpp_conn_new(m_ctx);

    // 'set account data to connection
    xmpp_conn_set_jid(m_conn, jid.c_str());
    xmpp_conn_set_pass(m_conn, pw.c_str());

    // connect
    return (xmpp_connect_client(m_conn, nullptr, 0, Xmpp::connect_handler, m_ctx) == 0);
}

void Xmpp::connect_handler(xmpp_conn_t* const conn, const xmpp_conn_event_t status, const int error, xmpp_stream_error_t* const stream_error, void* const userdata)
{
    xmpp_ctx_t* ctx = static_cast<xmpp_ctx_t*>(userdata);

    // Test if we are connected
    if( status == XMPP_CONN_CONNECT)
    {
        cout << "Connected to Account" << endl;

        // add handler for an versoin info request
        xmpp_handler_add(conn, Xmpp::version_handler, "jabber:iq:version", "iq", nullptr, ctx);

        // add handler for an incomming message
        xmpp_handler_add(conn, Xmpp::message_handler, nullptr, "message", nullptr, ctx);

        // add handler for bob's
        xmpp_handler_add(conn, Xmpp::bob_handler, "urn:xmpp:bob", "iq", "get", ctx);

        // send initial <presence/> so that we appear online to contacts
        xmpp_stanza_t* pres = xmpp_stanza_new(ctx);
        xmpp_stanza_set_name(pres, "presence");
        xmpp_send(conn, pres);
        xmpp_stanza_release(pres);
    }
    else
    {
        cerr << "DEVUG: disconnected" << endl;
        xmpp_stop(ctx);
    }
}

int Xmpp::version_handler(xmpp_conn_t* const conn, xmpp_stanza_t* const stanza, void* const userdata)
{
    xmpp_stanza_t *reply, *query, *name, *version, *text;

    char* ns;
    xmpp_ctx_t* ctx = static_cast<xmpp_ctx_t*>(userdata);
    cout << "Handle version request from: " << xmpp_stanza_get_from(stanza) << endl;

    reply = xmpp_stanza_reply(stanza);
    xmpp_stanza_set_type(reply, "result");

    query = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(query, "query");
    ns = xmpp_stanza_get_ns(xmpp_stanza_get_children(stanza));
    if( ns)
    {
        xmpp_stanza_set_ns(query, ns);
    }

    name = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(name, "name");

    text = xmpp_stanza_new(ctx);
    xmpp_stanza_set_text(text, Mongo::get_instance()->read_one(BSONObj(), "jabber", "Chatty")["resource"].str().c_str());
    xmpp_stanza_add_child(name, text);
    xmpp_stanza_release(text);
    xmpp_stanza_add_child(query, name);
    xmpp_stanza_release(name);

    version = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(version, "version");

    text = xmpp_stanza_new(ctx);
    xmpp_stanza_set_text(text, "1.0");
    xmpp_stanza_add_child(version, text);
    xmpp_stanza_release(text);
    xmpp_stanza_add_child(query, version);
    xmpp_stanza_release(version);

    xmpp_stanza_add_child(reply, query);
    xmpp_stanza_release(query);

    xmpp_send(conn, reply);

    return 1;
}

int Xmpp::message_handler(xmpp_conn_t* const conn, xmpp_stanza_t* const stanza, void* const userdata)
{
    // do an echo
    string in;
    //xmpp_ctx_t* ctx = static_cast<xmpp_ctx_t*>(userdata);

    // if no body do nothing
    if( !xmpp_stanza_get_child_by_name(stanza, "body"))
        return 1;
    // if it is an error do nothing
    if( xmpp_stanza_get_type(stanza) && !in.compare("error"))
        return 1;

    in = xmpp_stanza_get_text(xmpp_stanza_get_child_by_name(stanza, "body"));

    cout << "Incoming message from " << xmpp_stanza_get_from(stanza) << ": " << in << endl;

    // Do something with the incoming message

    //send_image(conn, xmpp_stanza_get_from(stanza), "/home/jonas/Logo.jpg");
    if( in.find("update") != string::npos)
    {
      xmpp_stanza_t* msg;
      xmpp_ctx_t* ctx = xmpp_conn_get_context(conn);
      msg = xmpp_stanza_new(ctx);
      xmpp_stanza_set_name(msg, "message");
      xmpp_stanza_set_type(msg, "chat");
      xmpp_stanza_set_attribute(msg, "to", xmpp_stanza_get_from(stanza));

      parse_msg( ctx, "<span style='font-weight: bolder;'>update</span>", msg);
      xmpp_send(conn, msg);
      xmpp_stanza_release(msg);

      this_thread::sleep_for(chrono::seconds(1));

      function_type update = (*m_commands)["update"];
      update( 0, vector<string>(), xmpp_stanza_get_from(stanza));
    }
    else
     cout << in << endl;

    return 1;
}

int Xmpp::bob_handler(xmpp_conn_t* const conn, xmpp_stanza_t* const stanza, void* const userdata)
{
    cout << "iq(bob) get request" << endl;
    if( xmpp_stanza_get_child_by_name(stanza, "data"))
    {

        string cid = xmpp_stanza_get_attribute(xmpp_stanza_get_child_by_name(stanza, "data"), "cid");
        size_t id = stoi(cid.substr(Mongo::get_instance()->read_one(BSONObj(),"jabber", "Chatty")["resource"].str().length()+stuff.length()));
        cout << cid << ";" << id << endl;
        string fileName = m_bob[id];
        string encoded;
        xmpp_stanza_t* iq, *data, *dat;
        xmpp_ctx_t *ctx = (xmpp_ctx_t*)userdata;

        iq = xmpp_stanza_new(ctx);
        xmpp_stanza_set_name(iq, "iq");
        xmpp_stanza_set_attribute(iq, "to", xmpp_stanza_get_from(stanza));
        xmpp_stanza_set_type(iq, "result");
        xmpp_stanza_set_id(iq, xmpp_stanza_get_id(stanza));

        data = xmpp_stanza_new(ctx);
        xmpp_stanza_set_name(data, "data");
        xmpp_stanza_set_attribute(data, "cid", cid.c_str());
        xmpp_stanza_set_attribute(data, "max-age", "86400");
        xmpp_stanza_set_type(data, mime_type(fileName).c_str());

        dat = xmpp_stanza_new(ctx);
        new FileSource ( fileName.c_str(), true, new Base64Encoder( new StringSink(encoded)));
        xmpp_stanza_set_text(dat, encoded.c_str());

        xmpp_stanza_add_child(data,dat);
        xmpp_stanza_release(dat);
        xmpp_stanza_add_child(iq, data);
        xmpp_stanza_release(data);

        xmpp_send(conn, iq);
        xmpp_stanza_release(iq);
    }
    return 1;
}

void Xmpp::add_elements(xmpp_ctx_t *ctx, xmlNode* ele, xmpp_stanza_t* msg, string &plain_msg)
{
  xmlNode *cur_node = NULL;
  bool body = false;

  for (cur_node = ele; cur_node; cur_node = cur_node->next)
  {
    xmpp_stanza_t* temp = NULL;
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      //cout << "node type: Element, name: " << cur_node->name << endl; // stanza add
      if( strcmp((char*)cur_node->name, "body"))
      {
        temp = xmpp_stanza_new(ctx);
        xmpp_stanza_set_name(temp, (char*)cur_node->name);
      }
      else
      {
        temp = msg;
        body = true;
      }

      xmlAttr* attribute = cur_node->properties;
      while(attribute && attribute->name && attribute->children)
      {
        xmlChar* value = xmlNodeListGetString(cur_node->doc, attribute->children, 1);
        //do something with value
        xmpp_stanza_set_attribute(temp, (char*)attribute->name, (char*)value);
        xmlFree(value);
        attribute = attribute->next;
      }
    }
    else if( xmlNodeIsText(cur_node))
    {
      string content;
      content = (char*)cur_node->content;
      plain_msg.append(content.c_str());
      xmpp_stanza_t* text = xmpp_stanza_new(ctx);
      xmpp_stanza_set_text(text, content.c_str());
      xmpp_stanza_add_child(msg, text);
      xmpp_stanza_release(text);
    }

    if( temp != NULL)
    {
      add_elements(ctx, cur_node->children, temp, plain_msg);
      if( !body)
      {
        xmpp_stanza_add_child(msg, temp);
        xmpp_stanza_release(temp);
      }
    }
    else
      add_elements(ctx, cur_node->children, msg, plain_msg);
  }
}

void Xmpp::parse_msg(xmpp_ctx_t *ctx, const char* const message, xmpp_stanza_t* msg)
{
  string content = "<body>";
  content += message;
  content += "</body>";
  string plain_msg;

  xmlDocPtr doc;
  xmlNode* root;

  doc = xmlReadMemory(content.c_str(), content.length(), "noname.xml", NULL, 0);

  if( doc == NULL)
  {
    cerr << "Failed to parse document" << endl;
    return;
  }


  root = xmlDocGetRootElement(doc);

  xmpp_stanza_t *html, *body, *plain, *text;

  html = xmpp_stanza_new(ctx);
  xmpp_stanza_set_name(html, "html");
  xmpp_stanza_set_ns(html, "http://jabber.org/protocol/xhtml-im");

  body = xmpp_stanza_new(ctx);
  xmpp_stanza_set_name(body, "body");
  xmpp_stanza_set_ns(body, "http://www.w3.org/1999/xhtml");

  add_elements(ctx, root, body, plain_msg);

  plain = xmpp_stanza_new(ctx);
  xmpp_stanza_set_name(plain, "body");
  text = xmpp_stanza_new(ctx);
  xmpp_stanza_set_text(text, plain_msg.c_str());
  xmpp_stanza_add_child(plain, text);
  xmpp_stanza_add_child(msg, plain);
  xmpp_stanza_release(text);
  xmpp_stanza_release(plain);

  xmpp_stanza_add_child(html, body);
  xmpp_stanza_release(body);
  xmpp_stanza_add_child(msg, html);
  xmpp_stanza_release(html);

  xmlFreeDoc(doc);


  xmlCleanupParser();
  xmlMemoryDump();
}

void Xmpp::send_image( xmpp_conn_t* const conn, string from, string file)
{
    xmpp_stanza_t* msg;
    xmpp_ctx_t* ctx;

    ctx = xmpp_conn_get_context(conn);

    msg = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(msg, "message");
    xmpp_stanza_set_type(msg, "chat");
    xmpp_stanza_set_attribute(msg, "to", from.c_str());

    auto index = find(m_bob.begin(), m_bob.end(), file);
    size_t pos;
    if( index == m_bob.end())
    {
        m_bob.push_back(file);
        pos = m_bob.size() -1;
    }
    else
    {
        pos = distance(m_bob.begin(), index);
    }


    string image = "<img src='cid:" + Mongo::get_instance()->read_one(BSONObj(),"jabber", "Chatty")["resource"].str() + stuff + to_string(pos) + "' />";

    cout << Mongo::get_instance()->read_one(BSONObj(),"jabber", "Chatty")["resource"].str() + stuff + to_string(pos) << ";" << to_string(pos) << endl;

    parse_msg(ctx, image.c_str(), msg);

    xmpp_send(conn, msg);
    xmpp_stanza_release(msg);
}

void Xmpp::run()
{
    xmpp_run(m_ctx);
}
