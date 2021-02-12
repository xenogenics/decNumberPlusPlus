/* ------------------------------------------------------------------ */
/* decNumber++ example program 2                                      */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2006.  All rights reserved.         */
/*                                                                    */
/* This software is made available under the terms of the IBM         */
/* alphaWorks License Agreement (distributed with this software as    */
/* alphaWorks-License.txt).  Your use of this software indicates      */
/* your acceptance of the terms and conditions of that Agreement.     */
/*                                                                    */
/* Please send comments, suggestions, and corrections to the author:  */
/*   klarer@ca.ibm.com                                                */
/*   Robert Klarer                                                    */
/* ------------------------------------------------------------------ */

#include <algorithm>
#include <functional>
#include <iostream>

#include "cdecmath"
#include "decimal"

// Given the length of its sides, this program computes the area of
// a triangle.

int main()
        {
        using std::decimal::decimal128;
        using std::decimal::sqrt;

        const unsigned int nSides = 3;
        decimal128 sides[3];

        // input the triangle's dimensions
        //
        for (int index = 0; index < nSides; ++index)
                {
                std::cout << "Enter the length of side "
                          << index << " of the triangle: "
                          << std::endl;
                std::cin >> sides[index];
                if (!std::cin.good() || sides[index] <= 0)
                        {
                        std::cerr << "Invalid input." << std::endl;
                        return 1;
                        }
                }

        // make sure that this is a valid triangle
        //
        std::sort(&sides[0], &sides[nSides]);
        if (sides[2] > (sides[0] + sides[1]))
                {
                        std::cerr << "Invalid input." << std::endl;
                        return 1;
                }

        // compute the area by Heron's formula
        //
        const decimal128 s = (sides[0] + sides[1] + sides[2]) / 2;
        const decimal128 area =
                sqrt(s * (s - sides[0]) * (s - sides[1]) * (s - sides[2]));

        std::cout << "The area of your triangle is " << area << std::endl;
        }

