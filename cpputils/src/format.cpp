#include <cpputils/format.h>

using namespace cpputils;

// This function is used to format a string using a format string and a list of arguments
// The string could look like this: "Hello, {}! You are \{{}\} years old."
// and arguments could be: "World", "25"
// The result would be: "Hello, World! You are {25} years old."
// We would neglect any other parameters passed but if there are less params then we would throw error
string cpputils::format_str(const string &fmt, const array_list<string> &args)
{
    // Create a buffer
    std::stringstream buffer;

    // Loop through the format string
    bool escaped = false;

    // Go through the list of arguments
    for (int i = 0, counter = 0; i < fmt.size(); i++)
    {
        // Get the character
        char c = fmt[i];
        // Check if we got an escaped character
        if (c == '\\' && !escaped)
        {
            // Set the escaped flag
            escaped = true;
        }
        // Check if we have a placeholder
        else if (c == '{' && !escaped)
        {
            // Move to the next character
            i++;
            // Check if we reached the end of the string
            if (i >= fmt.size())
            {
                // Throw an error
                throw std::runtime_error("Invalid format string");
            }

            // Check if we have a closing bracket
            if (fmt[i] != '}')
            {
                // Throw an error
                throw std::runtime_error("Invalid format string");
            }

            // Check if we have arguments
            if (args.size() == 0)
            {
                // Throw an error
                throw std::runtime_error("No arguments provided");
            }

            // Append the argument to the buffer
            buffer << args[counter];

            // Move to the next argument
            counter++;
        }
        else
        {
            // Append the character to the buffer
            buffer << c;
            // Reset the escaped flag
            escaped = false;
        }
    }
    // Return the buffer
    return buffer.str();
}