// Léon Zhou 02-2023
// KnxFrameAnalyzer.hpp
// projet KNX
// feel free to use this code

// std libs
#include <iostream>

// custom libs
#include "header/SerialPortListener.hpp"
#include "header/KnxFrameAnalyzer.hpp"

int main()
{
    // create the analyzer
    KnxFrameAnalyzer analyzer;

    // create the listener
    SerialPortListener listener("/dev/pts/5", &analyzer);

    // start the listener
    listener.start();

    // wait for the user to press enter
    while (std::cin.get() != 'q')
    {
        std::cout << "Press q to quit" << std::endl;
        std::cout << "Press any other key to continue" << std::endl;
    }
    // stop the listener
    listener.stop();

    // wait for the listener to stop

    return 0;
}