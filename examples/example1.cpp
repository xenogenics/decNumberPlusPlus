/* ------------------------------------------------------------------ */
/* decNumber++ example program 1                                      */
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

#include <iostream>

#include "decimal"

int main() {
        using std::decimal::decimal128;

        const decimal128 avogadro = "6.0221419E23";
        const decimal128 massH2O  = "18.0105";
        const decimal128 massFe   = "55.847";
        
        std::decimal::decimal128 mass;

        std::cout << "Enter a mass (in grams): " << std::endl;
        std::cin >> mass;
        if (std::cin.good() && (mass > 0)) {
                std::cout << mass << " gram(s) of pure water contains "
                          << (mass / massH2O * avogadro)
                          << " molecules.\n";
                std::cout << mass << " gram(s) of pure iron contains "
                          << (mass / massFe * avogadro)
                          << " molecules." << std::endl;
        }
        else {
                std::cerr << "Invalid input." << std::endl;
        }
}

