// Léon Zhou 02-2023
// KnxFrameAnalyzer.hpp
//  projet KNX
//  feel free to use this code

#ifndef KNXFRAMEANALYZER_HPP
#define KNXFRAMEANALYZER_HPP
// std libs
#include <string>
#include <vector>

/*A class to store Knx Frame Data*/
/*All data are stored in binary as string*/
struct KnxFrameData
{
    // functions

    // binToInt
    /*converts a binary string to an integer*/
    int binToInt(std::string bin)
    {
        return std::stoi(bin, nullptr, 2);
    }
    /*converts a binary string of size > 8 to a string of integers*/
    std::string binToInt(std::string bin, int size)
    {
        std::string str = "";
        for (int i = 0; i < size; i++)
        {
            str += std::to_string(binToInt(bin.substr(i * 4, 4)));
        }
        return str;
    }

    // intToBin
    /*converts an integer to a binary string*/
    std::string __intToBin(int num)
    {
        // check if the number is negative
        if (num < 0)
        {
            return "0";
        }

        // check if the number is less than 16
        if (num < 10)
        {
            return std::to_string(num);
        }
        switch (num)
        {
        case 10:
            return "A";
            break;
        case 11:
            return "B";
            break;
        case 12:
            return "C";
            break;
        case 13:
            return "D";
            break;
        case 14:
            return "E";
            break;
        case 15:
            return "F";
            break;
        default:
            return "0";
            break;
        }
    }

    // binToHex
    /*converts a binary string to a hex string*/
    std::string binToHex(std::string bin)
    {
        // separate the binary string into 4 bit chunks
        std::string bin2 = bin.substr(4, 4);
        std::string bin1 = bin.substr(0, 4);

        // convert the 4 bit chunks to hex
        int int1 = binToInt(bin1);
        int int2 = binToInt(bin2);

        return __intToBin(int1) + __intToBin(int2);
    }

    // get
    /*returns the data contained in this structure as a string*/
    /*separator can be changed via the parameter*/
    /*by default the separator is a space*/
    std::string get(std::string separator = " ")
    {
        std::string str = m_repeat + separator;
        str += m_priority + separator;
        str += m_source.get(separator) + separator;
        str += m_destination.get(separator) + separator;
        str += m_addrType + m_hopCount + m_length + separator;
        str += m_data + separator;
        str += m_checksum + separator;
        str += m_end;
        return str;
    }

    // refactor the frame
    /*refactors the frame*/
    /*return a string with separatord inbetween each bytes set as an argument*/
    /*by default the separator is a space*/
    /*the base can be changed via the parameter bin, dec, hex by default it is bin*/
    std::string refactor(std::string separator = " ", std::string base = "bin")
    {
        std::string str = "";
        if (base == "bin")
        {
            str = "10" + m_repeat + "1" + m_priority + "00" + separator;
            str += m_source.area + m_source.line + separator;
            str += m_source.device + separator;
            str += m_destination.area + m_destination.line + separator;
            str += m_destination.device + separator;
            str += m_addrType + m_hopCount + m_length + separator;
            str += m_data + separator;
            str += m_checksum + separator;
            str += m_end;
        }
        else if (base == "dec")
        {
            str = std::to_string(binToInt("10" + m_repeat + "1" + m_priority + "00")) + separator;
            str += std::to_string(binToInt(m_source.area + m_source.line)) + separator;
            str += std::to_string(binToInt(m_source.device)) + separator;
            str += std::to_string(binToInt(m_destination.area + m_destination.line)) + separator;
            str += std::to_string(binToInt(m_destination.device)) + separator;
            str += std::to_string(binToInt(m_addrType + m_hopCount + m_length)) + separator;
            str += binToInt(m_data, binToInt(m_length) + 1) + separator;
            str += std::to_string(binToInt(m_checksum)) + separator;
            str += std::to_string(binToInt(m_end));
        }
        else if (base == "hex")
        {
            str = binToHex("10" + m_repeat + "1" + m_priority + "00") + separator;
            str += binToHex(m_source.area + m_source.line) + separator;
            str += binToHex(m_source.device) + separator;
            str += binToHex(m_destination.area + m_destination.line) + separator;
            str += binToHex(m_destination.device) + separator;
            str += binToHex(m_addrType + m_hopCount + m_length) + separator;
            str += binToHex(m_data) + separator;
            str += binToHex(m_checksum) + separator;
            str += binToHex(m_end);
        }
        return str;
    }

    // datatypes
    struct KnxFrameDataAddress
    {
        std::string area;
        std::string line;
        std::string device;

        // functions
        /*returns the data contained in this structure as a string*/
        /*separator can be changed via the parameter*/
        /*by default the separator is a space*/
        std::string get(std::string separator = " ")
        {
            std::string str = area + line + separator + device;
            return str;
        }
    };

    std::string m_repeat;
    std::string m_priority;
    KnxFrameDataAddress m_source;
    KnxFrameDataAddress m_destination;
    std::string m_addrType;
    std::string m_hopCount;
    std::string m_length;
    std::string m_data;
    std::string m_checksum;
    std::string m_end;
};

/*A class that stores and scrape its data*/
class KnxFrameAnalyzer
{
private:
    std::vector<KnxFrameData> m_completeData;
    std::vector<std::string> m_frame;
    KnxFrameData m_data;
    int stage = 0;
    bool newData = false;

    // functions
    /*byte to bin*/
    std::string byteToBin(char byte);

    // methods for scraping
    // scrapes the data
    /*control byte*/
    bool scrapeByteOne(); /*1*/
    /*sender addr area & line*/
    bool scrapeByteTwo(); /*2*/
    /*sender addr device*/
    bool scrapeByteThree(); /*3*/
    /*receiver addr area & line*/
    bool scrapeByteFour(); /*4*/
    /*receiver addr device*/
    bool scrapeByteFive(); /*5*/
    /*addr type & hop count & length*/
    bool scrapeByteSix(); /*6*/
    /*data*/
    bool scrapeByteData(); /*7*/
    /*checksum*/
    bool scrapeByteChecksum(); /*8*/
    /*end*/
    bool scrapeByteEnd(); /*9*/

    /*returns true if the operation is complete*/
    bool scrapeComplete();

    /*do one complete operation*/
    /*called by scrapeComplete()*/
    bool doSomething();

public:
    bool addData(char data);
    bool addData(char *data, int size);
    bool addData(int data);

    bool analyze();

    KnxFrameData getData(bool erase = false);

    /*deletes all the data*/
    void purge();

    /*return true if there is new data*/
    bool hasNewData();

    /*return true is there's data*/
    bool hasData();

    /*returns an instance os the current data*/
    KnxFrameData getCurrentData();

    // constructors & destructors
    KnxFrameAnalyzer();
    ~KnxFrameAnalyzer();
};

#endif // KNXFRAMEANALYZER_HPP