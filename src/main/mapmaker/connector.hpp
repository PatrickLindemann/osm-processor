#pragma once

#include <vector>

#include "model/geometry/segment.hpp";

namespace mapmaker
{

    namespace connector
    {

        using namespace model;

        template <typename T>
        class Connector
        {
        public:

        protected:

        public:

            Connector();

            std::vector<geometry::Segment<T>> connections()
            {

            };

        protected:

            void run()
            {

            }

        };

    }

}