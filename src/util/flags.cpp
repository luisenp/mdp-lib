#include <cassert>

#include "../../include/util/flags.h"
#include "../../include/util/general.h"

using namespace std;

namespace mdplib
{

unordered_map<string, string> FLAGS;

void register_flags(int argc, char* args[])
{
    for (int i = 0; i < argc; i++) {
        if (args[i][0] == '-' && args[i][1] == '-') {
            string argument(args[i]);
            string flag, value;
            size_t pos_equals = argument.find("=");
            if (pos_equals != string::npos) {
                flag = argument.substr(2, pos_equals - 2);
                value = argument.substr(pos_equals + 1,
                                        argument.size() - pos_equals);
            }
            else {
                flag = argument.substr(2, argument.size() - 1);
                value = "";
            }
            FLAGS[flag] = value;
        }
    }
}

string flag_value(string flag_name)
{
    if (FLAGS.count(flag_name))
        return FLAGS[flag_name];
    else
        return "";
}

bool flag_is_registered(string flag)
{
    return FLAGS.count(flag) > 0;
}

bool flag_is_registered_with_value(string flag)
{
    return flag_is_registered(flag) && FLAGS[flag] != "";
}

}


