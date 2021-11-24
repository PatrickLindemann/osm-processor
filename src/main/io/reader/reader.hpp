#pragma once

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace io
{

    /**
     * A virtual reader class.
     */
    template <typename T>
    class Reader
    {
    protected:

        /* Members */
        
        /**
         * The input file path.
         */
        fs::path m_path;

        /* Constructors */

        /**
         * Creates a reader for a specified input file.
         * 
         * @param file_path The input file path.
         */
        Reader(fs::path file_path) : m_path(file_path) {}

    public:

        /* Virtual Methods */

        /**
         * Reads the contents of the input file and returns the result
         * object.
         * 
         * @returns The result object
         * @throws  Exception if the file does not exist or is invalid
         */
        virtual T read() = 0;

    };

}