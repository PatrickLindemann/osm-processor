#ifndef UTIL_TABLE_HPP
#define UTIL_TABLE_HPP

#include <map>
#include <tuple>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

#include <boost/lexical_cast.hpp>

namespace util
{

    template <typename ...T>
    class Table
    {
    public:

        /* Types */

        using data_type = std::tuple<T...>;

    protected:

        /* Members */

        /**
         * 
         */
        std::vector<std::string> m_headers;

        /**
         * 
         */
        std::vector<data_type> m_rows;

        /**
         * 
         */
        std::vector<size_t> m_col_widths;

        /**
         * 
         */
        size_t m_total_width;

    public:

        /* Constructors */

        /**
         * 
         */
        Table(const std::vector<std::string>& headers) : m_headers(headers)
        {
            if (headers.size() != sizeof...(T))
            {
                throw std::invalid_argument("Number of headers doesn't match the number of columns.");
            }
        };

        /**
         * 
         */
        template <typename K, typename V>
        Table(const std::vector<std::string>& headers, const std::map<K, V>& map) : m_headers(headers)
        {
            if (headers.size() != 2)
            {
                throw std::invalid_argument("Number of headers has to be 2 for tables from maps.");
            }
            for (const auto& entry : map)
            {
                add_row(entry.first, entry.second);
            }
        };

        /**
         * 
         */
        template <typename K, typename V>
        Table(const std::vector<std::string>& headers, const std::unordered_map<K, V>& map) : m_headers(headers)
        {
            if (headers.size() != 2)
            {
                throw std::invalid_argument("Number of headers has to be 2 for tables from maps.");
            }
            for (const auto& entry : map)
            {
                add_row(entry.first, entry.second);
            }
        };

        /* Methods */

        /**
         * 
         */
        const bool empty() const
        {
            return m_rows.empty();
        }

        /**
         * 
         */
        const size_t column_count() const
        {
            return m_headers.size();
        }

        /**
         * 
         */
        const size_t row_count() const
        {
            return m_rows.size();
        }

        /**
         * 
         */
        void add_row(T... entry)
        {
            m_rows.push_back(std::make_tuple(entry...));
        }

    protected:
        
    /* Helpers */

        /**
         * 
         */
        template <typename ValueType>
        size_t length(const ValueType& value) const
        {
            return boost::lexical_cast<std::string>(value).length();
        }

        /**
         * Recursion end
         */
        template <typename TupleType>
        void size_columns(TupleType&&, std::vector<size_t>& col_widths, std::integral_constant<
            size_t,
            std::tuple_size<typename std::remove_reference<TupleType>::type>::value>
        ) {
            return;
        }

        /**
         * Recursion step
         */
        template <size_t I, typename TupleType,
            typename = typename std::enable_if<I != std::tuple_size<typename std::remove_reference<TupleType>::type>::value>::type>
        void size_columns(TupleType&& tuple, std::vector<size_t>& col_widths, std::integral_constant<size_t, I>)
        {
            col_widths.at(I) = std::max(col_widths.at(I), length(std::get<I>(tuple)));
            size_columns(
                std::forward<TupleType>(tuple),
                col_widths,
                std::integral_constant<size_t, I + 1>()
            );
        }

        /**
         * Recursion start
         */
        template <typename TupleType>
        void size_columns(TupleType&& tuple, std::vector<size_t>& col_widths)
        {
            size_columns(
                std::forward<TupleType>(tuple),
                col_widths,
                std::integral_constant<size_t, 0>()
            );
        }

        /**
         * 
         */
        void resize()
        {
            // Determine the width for each column
            for (size_t i = 0; i < column_count(); i++)
            {
                m_col_widths.push_back(m_headers.at(i).length());
            }
            for (data_type& row : m_rows)
            {
                size_columns(row, m_col_widths);
            }

            // Determine the total width
            // Each column gets printed as | content |, so the total
            // width is #cols * (width[col] + 2 * ' ' + 1 * '|') + 1
            m_total_width = 0;
            for (size_t col_width : m_col_widths)
            {
                m_total_width += col_width + 3;
            }
            m_total_width += 1;
        }

        /* Printing Helpers */

        /**
         * Recursion end
         */
        template <typename TupleType, typename StreamType>
        void print_cells(TupleType&&, StreamType&, std::integral_constant<
            size_t,
            std::tuple_size<typename std::remove_reference<TupleType>::type>::value>
        ) {
            return;
        }

        /**
         * Recursion step
         */
        template <size_t I, typename TupleType, typename StreamType,
            typename = typename std::enable_if<I != std::tuple_size<typename std::remove_reference<TupleType>::type>::value>::type>
        void print_cells(TupleType&& tuple, StreamType& stream, std::integral_constant<size_t, I>)
        {
            // Print current tuple value
            std::string cell = boost::lexical_cast<std::string>(std::get<I>(tuple));
            stream << ' '
                << cell
                << std::string(m_col_widths.at(I) - cell.length(), ' ')
                << ' '
                << '|';
            // Print next value
            print_cells(std::forward<TupleType>(tuple), stream, std::integral_constant<size_t, I + 1>());
        }

        /**
         * Recursion start
         */
        template <typename TupleType, typename StreamType>
        void print_cells(TupleType&& tuple, StreamType& stream)
        {
            print_cells(std::forward<TupleType>(tuple), stream, std::integral_constant<size_t, 0>());
        }

    public:

        /* Printing methods */

        /**
         * 
         */
        template <typename StreamType>
        void print(StreamType& stream)
        {
            // Resize table
            resize();

            // Print the top line
            stream << std::string(m_total_width, '-') << '\n';

            // Print the headers (left-aligned)
            stream << '|';
            for (size_t i = 0; i < column_count(); i++)
            {
                stream << ' '
                    << m_headers.at(i)
                    << std::string(m_col_widths.at(i) - m_headers.at(i).length(), ' ')
                    << ' '
                    << '|';
            }
            stream << '\n';

            // Print the divider
            stream << std::string(m_total_width, '-') << '\n';

            if (!m_rows.empty())
            {
                // Print the rows (left-aligned)
                for (data_type& row : m_rows)
                {
                    stream << '|';
                    print_cells(row, stream);
                    stream << '\n';
                }
            }
            else
            {
                // Print an empty row
                stream << '|' << std::string(m_total_width - 2, ' ') << '|' << '\n'; 
            }

            // Print the bottom line
            stream << std::string(m_total_width, '-') << std::endl;
        }

    };

}

#endif