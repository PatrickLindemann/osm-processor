#pragma once

#include <cmath>

namespace mapmaker
{

    namespace connector
    {

        template <typename T>
        class Calculator
        {
        public:

        protected:

        public:

            Calculator(
            );

        protected:

            const double PRECISION_MIN = std::pow(10, -4);
            const double PRECISION_MIN_REVERSE = 1 / PRECISION_MIN;


            void get_centerpoints();

            /**
             * 
             * @param area
             * @param total
             * @param k
             * @returns 
             */
            double get_precision(double area, double total, double k = 25)
            {
                return area / ( 1 + std::exp(-k * area / total) * (PRECISION_MIN_REVERSE - 1));
            }

        public:

            void run()
            {

            }

        };

    }

}