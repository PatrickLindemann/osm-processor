#pragma once

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace io
{

    /**
     * A virtual writer class.
     */
    template <typename T>
    class Writer
    {
    protected:

        /* Members */
        
        /**
         * The output file path.
         */
        fs::path m_path;

        /* Constructors */

        /**
         * Creates a writer for a specified output file.
         * 
         * @param file_path The output file path.
         */
        Writer(fs::path file_path) : m_path(file_path) {}

    public:

        /* Virtual Methods */

        /**
         * Writes the specified content to the output file.
         * 
         * @param content The data that is written to the output file
         * @throws        Exception if an exception occured during the writing
         *                process
         */
        virtual void write(T&& content) = 0;

    };

}