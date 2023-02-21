// Léon Zhou 02-2023
// KnxFrameAnalyzer.hpp
// projet KNX
// feel free to use this code

#ifndef LISTENER_HPP
#define LISTENER_HPP
#include <iostream>

// std lib
#include <string>
#include <functional>
#include <boost/asio.hpp>
#include <atomic>
#include <thread>

// custom lib
#include "KnxFrameAnalyzer.hpp"

// A listener class that reads data from a serial port
class SerialPortListener
{
private:
    // the serial port
    boost::asio::io_context *m_io;
    boost::asio::serial_port *m_port;

    // thread to run the io_context on
    std::thread m_thread;

    // atomic bool to indicate if the read operation should be cancelled
    std::atomic<bool> cancel_read_operation;
    bool m_is_running;

    // KnxFrameAnalyzer reference
    KnxFrameAnalyzer *m_analyzer;

    // functions

    // this function will the threaded.
    // it will read asynchronously from the serial port
    void thread_function();

public:
    // constructor, takes the serial port name and the KnxFrameAnalyzer reference
    SerialPortListener(const std::string &port_name, KnxFrameAnalyzer *analyzer);

    // destructor
    ~SerialPortListener();

    // start the listener
    void start();

    // stop the listener
    void stop();

    // check if the listener is running
    bool is_running() const;
};
#endif // LISTENER_HPP