//
// Created by jonas on 12.03.16.
//

#ifndef CHATTY_COMMAND_H
#define CHATTY_COMMAND_H

#include "commands.h"


class Command
{

  public:
    Command();
    virtual ~Command();

    void register_command(string name, function_type function);
    bool is_command( string command);
    function_type operator[](string name);

  private:
    map<string, function_type> m_commands;
};


#endif //CHATTY_COMMAND_H
