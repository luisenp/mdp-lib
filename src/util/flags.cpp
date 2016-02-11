#include <cassert>

#include "../../include/util/flags.h"
#include "../../include/util/general.h"

using namespace std;

namespace mdplib
{

unordered_map<string, string> FLAGS;

/**
 * Takes the command line arguments passed in args and registers all flags
 * present in the arguments.
 */
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


/**
 * Returns the value of the flag with the given name.
 * For example, if the command line argument --foo=bar was registered,
 * then a call to flag_value("foo") returns "bar".
 */
string flag_value(string flag_name)
{
    if (FLAGS.count(flag_name))
        return FLAGS[flag_name];
    else
        return "";
}

/**
 * Returns true is the flag was registered.
 */
bool flag_is_registered(string flag)
{
    return FLAGS.count(flag) > 0;
}

/**
 * Returns true if the flag was registered with a value (e.g. a command
 * line argument such as --foo=bar).
 */
bool flag_is_registered_with_value(string flag)
{
    return flag_is_registered(flag) && FLAGS[flag] != "";
}

}


