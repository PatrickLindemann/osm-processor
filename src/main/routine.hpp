#pragma once

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

/**
 * A base class for program routines.
 */
class Routine
{
protected:

    /* Members */

    /**
     * The current execution directory.
     */
    fs::path m_dir;

    /**
     * The program options for this routine.
     */
    po::options_description m_options;

    /**
     * The positional program options for this routine.
     */
    po::positional_options_description m_positional;

    /**
     * The variable map that stores the string values of each parameter.
     */
    po::variables_map m_variables;

    /* Constructors */

    Routine() : m_options("Allowed Options") {}

    /* Setters */

    template <typename T>
    void set(T* variable, std::string key)
    {
        *variable = m_variables.at(key).as<T>();
    }

    template <typename T>
    void set(T* variable, std::string key, T default_value)
    {
        if (!m_variables.count(key) || m_variables.at(key).defaulted())
        {
            *variable = default_value;
        }
        else
        {
            *variable = m_variables[key].as<T>();
        }
    }

    template <typename T>
    void set(T* variable, std::string key, std::function<void(T&, std::string)> validator)
    {
        set(variable, key);
        validator(*variable, key);
    }

    template <typename T>
    void set(T* variable, std::string key, T default_value, std::function<void(T&, std::string)> validator)
    {
        set(variable, key, default_value);
        validator(*variable, key);
    }

public:
    
    /* Accessors */

    const po::variables_map& variables() const noexcept
    {
        return m_variables;
    }

    /* Methods */

    /**
     * Print the help message for this routine.
     */
    void help() const noexcept
    {
        std::cout << m_options << std::endl; 
    };

    /**
     * Initialize this routine with the specified command line arguments.
     *
     * @param argc The command line argument count
     * @param argv The command line argument values
     */
    void init(int argc, char* argv[])
    {
        // Extract root dir from argv and remove the command from it
        m_dir = fs::system_complete(fs::path(argv[0])).parent_path();
        argc--;
        argv++;

        // Prepare the command line parser that uses the positional and optional
        // options of this command to extract the parameters
        po::command_line_parser parser(argc, argv);
        parser.options(m_options);
        parser.positional(m_positional);

        // Run the parser and store the parameters in the specified variables and 
        // the prepared variable map
        po::variables_map vm;
        po::store(parser.run(), vm);
        m_variables = vm;
    }

    /* Virtual Methods */

    /**
     * Retrieve the name of this routine.
     */
    virtual const std::string name() const noexcept = 0;

    /**
     * Setup and validate the specified parameters.
     * @throws Exception if a parameter is invalid.
     */
    virtual void setup()
    {
        po::notify(m_variables);
    };

    /**
     * Execute this routine with the parameters specified in the setup. 
     */
    virtual void run() = 0;
    
};