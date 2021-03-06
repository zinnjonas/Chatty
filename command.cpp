//
// Created by jonas on 12.03.16.
//

#include "command.h"

Command::Command()
{
}

Command::~Command()
{
}

void Command::register_command( string name, function_type function )
{
  m_commands[name] = function;
}

bool Command::is_command( string command )
{

  return m_commands.find(command) != m_commands.end();;
}

function_type Command::operator[](string command)
{
  return m_commands[command];
}

