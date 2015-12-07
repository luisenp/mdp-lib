/**
 * This file defines methods to handle command line flags.
 */

#include <unordered_map>

namespace mdplib
{

extern std::unordered_map<std::string, std::string> FLAGS;

/**
 * Registers the flags present in the commnad line arguments
 * passed in argv.
 * A flag is any command line argument prefixed by "--".
 * Flags may or may not have a value. If they have a value, the
 * corresponding command line argument has the format "--flag=value".
 * Otherwise, the format is "--flag".
 *
 * The method adds all flags to the FLAGS set.
 *
 * @param argc the number of command line arguments.
 * @param argv the command line arguments.
 */
void register_flags(int argc, char* args[]);

/**
 * Returns the value of a flag.
 *
 * @param flag_name the flag's name.
 * @return the flag's value or an empty string if the flag
 *        is not present or has no value.
 */
std::string flag_value(std::string flag_name);

/**
 * Returns true if the flag was registered.
 *
 * @param flag the flag to check if it is registered.
 * @return true if the flag is registered, false otherwise.
 */
bool flag_is_registered(std::string flag);

/**
 * Returns true if the flag was registered with a value.
 *
 * @param flag the flag to check if it is registered.
 * @return true if the flag is registered, false otherwise.
 */
bool flag_is_registered_with_value(std::string flag);

}


