#include "../header/SerialPortListener.hpp"

/*construcor*/
SerialPortListener::SerialPortListener(const std::string &port_name, KnxFrameAnalyzer *analyzer)
{
	// setup the serial port
	m_io = new boost::asio::io_context();

	// setup the serial port
	m_port = new boost::asio::serial_port(*m_io, port_name);

	// set options
	m_port->set_option(boost::asio::serial_port_base::baud_rate(9600));
	m_port->set_option(boost::asio::serial_port_base::character_size(8));
	m_port->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
	m_port->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
	m_port->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

	// set the analyzer reference
	m_analyzer = analyzer;

	// set the cancel_read_operation to false
	cancel_read_operation = false;

	// set the is_running to false
	m_is_running = false;
}

/*destructor*/
SerialPortListener::~SerialPortListener()
{
	// stop the listener
	if (m_is_running)
	{
		stop();
	}

	// delete the serial port
	delete m_port;

	// delete the io_context
	delete m_io;
}

/*stop the listener*/
void SerialPortListener::stop()
{
	// set the cancel_read_operation to true
	cancel_read_operation = true;

	m_thread.join();
}

/*start the listener*/
void SerialPortListener::start()
{
	// set the cancel_read_operation to false
	cancel_read_operation = false;

	// start the thread
	m_thread = std::thread(&SerialPortListener::thread_function, this);

	// set the is_running to true
	m_is_running = true;
}

/*check if the listener is running*/
bool SerialPortListener::is_running() const
{
	return m_is_running;
}

/*this function will the threaded.
it will read asynchronously from the serial port
the timeout is set to 1 seconds*/
void SerialPortListener::thread_function()
{
	// create a timer
	boost::asio::deadline_timer timer(*m_io);

	// create a buffer
	std::vector<char> data(1024);
	boost::asio::mutable_buffer buffer = boost::asio::mutable_buffer(data.data(), data.size());

	// start the async read operation
	while (!cancel_read_operation)
	{

		// start the async read operation
		m_port->async_read_some(buffer, [&](const boost::system::error_code &error, std::size_t bytes_transferred)
								{
         // we don't care about the error code
         if (!error)
         {
             // add the data to the analyzer
             m_analyzer->addData(data.data(), bytes_transferred);

             // analyze the data
             m_analyzer->analyze();
         } });

		// set a timeout for the read operation
		timer.expires_from_now(boost::posix_time::seconds(1));
		// lambda function
		timer.async_wait([&](const boost::system::error_code &error)
						 {
            if (error != boost::asio::error::operation_aborted)
            {
                m_port->cancel();
            } });

		// run the io_context
		m_io->run();

		// reset the io_context
		m_io->reset();

		// reset the timer
		timer.cancel();

		// reset the buffer
		buffer = boost::asio::mutable_buffer(data.data(), data.size());
	}

	// stop all operations
	data.clear();

	// close the serial port
	m_port->close();

	// stop the io_context
	m_io->stop();

	// set the is_running to false
	m_is_running = false;
}