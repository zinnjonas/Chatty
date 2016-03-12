//
// Created by jonas on 12.03.16.
//

#ifndef CHATTY_COMMANDS_H
#define CHATTY_COMMANDS_H

#include <string>
#include <vector>
#include <map>

using namespace std;

typedef int(*function_type)(int,vector<string>, string);

int update(int argc, vector<string>, string from);


#endif //CHATTY_COMMANDS_H
