#include "../header/KnxFrameAnalyzer.hpp"

// constructor
KnxFrameAnalyzer::KnxFrameAnalyzer()
{
}
// destructor
KnxFrameAnalyzer::~KnxFrameAnalyzer()
{
    // clear all data
    purge();
}

// functions
std::string KnxFrameAnalyzer::byteToBin(char byte)
{
    std::string bin;
    for (int i = 7; i >= 0; i--)
    {
        if ((byte >> i) & 1)
        {
            bin += "1";
        }
        else
        {
            bin += "0";
        }
    }
    return bin;
}

// methods
/*adds one byte*/
bool KnxFrameAnalyzer::addData(char data)
{
    m_frame.push_back(byteToBin(data));
    return true;
}

/*adds a buffer of bytes*/
bool KnxFrameAnalyzer::addData(char *data, int size)
{
    if (size < 1)
    {
        return false;
    }

    for (int i = 0; i < size; i++)
    {
        m_frame.push_back(byteToBin(data[i]));
    }
    return true;
}

/*deletes all data contained in this class*/
void KnxFrameAnalyzer::purge()
{
    m_frame.clear();
    m_data = KnxFrameData();
    m_completeData.clear();
    stage = 0;
    newData = false;
}

// getters
/*Put true in arg to delete this data*/
/*this returns a datastructure containing all the data*/
KnxFrameData KnxFrameAnalyzer::getData(bool erase)
{
    if (m_completeData.size() == 0)
    {
        return KnxFrameData();
    }
    else if (erase)
    {
        KnxFrameData data = m_completeData[0];
        m_completeData.erase(m_completeData.begin());
        return data;
    }

    newData = false;
    return m_completeData[0];
}

/*returns true if there is new data*/
bool KnxFrameAnalyzer::hasNewData()
{
    if (newData)
    {
        newData = false;
        return true;
    }
    return false;
}

// return true if has data
bool KnxFrameAnalyzer::hasData()
{
    if (m_completeData.size() > 0)
    {
        return true;
    }
    return false;
}

// analyzer
// scraping methods

// control byte
bool KnxFrameAnalyzer::scrapeByteOne()
{
    if (m_frame.size() < 1)
    {
        return false;
    }
    // 3rd bit of first byte is the repeat bit
    m_data.m_repeat = m_frame[0][2];

    // 5-6th bit of first byte are the priority
    m_data.m_priority = m_frame[0].substr(4, 2);

    // pop this byte
    m_frame.erase(m_frame.begin());

    // operation complete
    stage = 1;

    // call the next function
    return scrapeByteTwo();
}

// sender addr area & line
bool KnxFrameAnalyzer::scrapeByteTwo()
{
    if (m_frame.size() < 1)
    {
        return false;
    }
    // 1-4th bit of first byte are the area
    m_data.m_source.area = m_frame[0].substr(0, 4);
    // 5-8th bit of first byte are the line
    m_data.m_source.line = m_frame[0].substr(4, 4);

    // pop this byte
    m_frame.erase(m_frame.begin());

    // operation complete
    stage = 2;

    // call the next function
    return scrapeByteThree();
}

// sender addr device
bool KnxFrameAnalyzer::scrapeByteThree()
{
    if (m_frame.size() < 1)
    {
        return false;
    }
    // 1-8th bit of first byte are the device
    m_data.m_source.device = m_frame[0];

    // pop this byte
    m_frame.erase(m_frame.begin());

    // operation complete
    stage = 3;

    // call the next function
    return scrapeByteFour();
}

// receiver addr area & line
bool KnxFrameAnalyzer::scrapeByteFour()
{
    if (m_frame.size() < 1)
    {
        return false;
    }
    // 1-4th bit of first byte are the area
    m_data.m_destination.area = m_frame[0].substr(0, 4);
    // 5-8th bit of first byte are the line
    m_data.m_destination.line = m_frame[0].substr(4, 4);

    // pop this byte
    m_frame.erase(m_frame.begin());

    // operation complete
    stage = 4;

    // call the next function
    return scrapeByteFive();
}

// receiver addr device
bool KnxFrameAnalyzer::scrapeByteFive()
{
    if (m_frame.size() < 1)
    {
        return false;
    }
    // 1-8th bit of first byte are the device
    m_data.m_destination.device = m_frame[0];

    // pop this byte
    m_frame.erase(m_frame.begin());

    // operation complete
    stage = 5;

    // call the next function
    return scrapeByteSix();
}

// addr type & hop count & length
bool KnxFrameAnalyzer::scrapeByteSix()
{
    if (m_frame.size() < 1)
    {
        return false;
    }
    // 1st bit of first byte is the addr type
    m_data.m_addrType = m_frame[0][0];

    // 2-4th bit of first byte are the hop count
    m_data.m_hopCount = m_frame[0].substr(1, 3);

    // 5-8th bit of first byte are the length
    m_data.m_length = m_frame[0].substr(4, 4);

    // pop this byte
    m_frame.erase(m_frame.begin());

    // operation complete
    stage = 6;

    // call the next function
    return scrapeByteData();
}

// data byte
bool KnxFrameAnalyzer::scrapeByteData()
{
    if (m_frame.size() < 1)
    {
        return false;
    }
    // if there isn't enough data in the frame
    // then we won't go to the next stage
    bool reRun = false;
    // first we need to know how many bytes we need to read
    // we have to add 1 to length as 0 is a length of 1
    int length = m_data.binToInt(m_data.m_length) + 1;
    // if there is data stored then it's in bits so we need to divide by 8
    // this is the number of bytes we have to read
    int remaining = length - m_data.m_data.size() / 8;

    // if the remaining bytes are more than the bytes in the frame
    // then we need to read all the bytes in the frame
    if (remaining > m_frame.size())
    {
        remaining = m_frame.size();
        reRun = true;
    }

    // read the remaining bytes
    for (int i = 0; i < remaining; i++)
    {
        m_data.m_data += m_frame[0];
        m_frame.erase(m_frame.begin());
    }

    // if we need to rerun this function then we don't go to the next stage
    if (reRun)
    {
        // operation incomplete as we need to read more data
        return false;
    }

    // operation complete
    stage = 7;

    // call the next function
    return scrapeByteChecksum();
}

// checksum
bool KnxFrameAnalyzer::scrapeByteChecksum()
{
    if (m_frame.size() < 1)
    {
        return false;
    }
    m_data.m_checksum = m_frame[0];
    m_frame.erase(m_frame.begin());

    // operation complete
    stage = 8;

    // call the next function
    return scrapeByteEnd();
}

// end byte
bool KnxFrameAnalyzer::scrapeByteEnd()
{
    if (m_frame.size() < 1)
    {
        return false;
    }
    m_data.m_end = m_frame[0];
    m_frame.erase(m_frame.begin());

    // operation complete
    stage = 9;

    // call the next function
    return scrapeComplete();
}

// returns true if the operation is complete
bool KnxFrameAnalyzer::scrapeComplete()
{
    if (stage == 9)
    {
        stage = 0;
        m_completeData.push_back(m_data);
        newData = true;
        m_data = KnxFrameData();

        doSomething();

        return true;
    }
    return false;
}

// this function is called by scrapeComplete
// it does something with the data
bool KnxFrameAnalyzer::doSomething()
{
    // do something with the data

    // i'll implement this later
    // it will be used to send the data to the database

    // end
    return true;
}

// using the prevous functions we can now scrape the frame
bool KnxFrameAnalyzer::analyze()
{
    // if there is no data in the frame then we can't do anything
    if (m_frame.size() < 1)
    {
        return false;
    }

    // we need to run the correct function based on the stage
    bool finished = false;
    switch (stage)
    {
    case 0:
        finished = scrapeByteOne();
        break;
    case 1:
        finished = scrapeByteTwo();
        break;
    case 2:
        finished = scrapeByteThree();
        break;
    case 3:
        finished = scrapeByteFour();
        break;
    case 4:
        finished = scrapeByteFive();
        break;
    case 5:
        finished = scrapeByteSix();
        break;
    case 6:
        finished = scrapeByteData();
        break;
    case 7:
        finished = scrapeByteChecksum();
        break;
    case 8:
        finished = scrapeByteEnd();
        break;
    default:
        break;
    }

    // if there sill is data in the frame then we need to run this function again recursively
    if (m_frame.size() > 0)
    {
        analyze();
    }

    // if the operation is complete then we can return true
    return finished;
}

// returns an instance os the current data
KnxFrameData KnxFrameAnalyzer::getCurrentData()
{
    return m_data;
}