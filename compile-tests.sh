#!/bin/bash
g++ -std=c++11 -Wall --pedantic-errors load-test.cpp -lpng -o load-test.exe
g++ -std=c++11 -Wall --pedantic-errors save-test.cpp -lpng -o save-test.exe
