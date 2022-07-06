#include "simpleini.h"
#include "gtest/gtest.h"

#include <iostream>
#include <limits>
#include <list>
#include <sstream>
#include <string>
#include <vector>


class TestWriter;
class TestReader;

using Config = simpleini::ConfigImpl<TestReader, TestWriter>;


class TestWriter
{
public:
    TestWriter(const std::string&)
    {
    }

    template <typename T>
    std::ostream& operator<<(const T& t)
    {
        return output << t;
    }

    bool is_open() const
    {
        return true;
    }

    static std::ostringstream output;
};

std::ostringstream TestWriter::output;

class TestReader
{
public:
    TestReader(const std::string&)
    {
    }

    bool getLine(std::string& line);

    static std::vector<std::string> input;

private:
    size_t m_position { 0 };
};

std::vector<std::string> TestReader::input {};

bool TestReader::getLine(std::string &line)
{
    if (m_position < TestReader::input.size())
    {
        line = input[m_position++];
        return true;
    }
    return false;
}

class ReadWrite : public testing::Test
{
protected:
    Config config;
    Config output;

    void SetUp() override;

    void writeAndRead(simpleini::SaveFlags flags)
    {
        TestReader::input.clear();
        TestWriter::output.str("");

        config.save("", flags);
        config = {};

        std::istringstream iss { TestWriter::output.str() };
        std::string line;
        while (std::getline(iss, line))
        {
            TestReader::input.push_back(line);
        }
        output = Config::load("");
    }
};

void ReadWrite::SetUp()
{
    config = {};
    output = {};
    TestReader::input.clear();
    TestWriter::output.str("");
}

class Read: public testing::Test
{
protected:
    std::vector<std::string>& file = TestReader::input;
    simpleini::ConfigImpl<TestReader, TestWriter> config;

    void SetUp() override;

    void load()
    {
        config = simpleini::ConfigImpl<TestReader, TestWriter>::load("");
    }
};

void Read::SetUp()
{
    TestReader::input.clear();
}

template <typename T>
bool compareArrays(const T& p, const T& q)
{
    if (p.size() != q.size())
    {
        return false;
    }

    for (size_t i = 0; i < p.size(); ++i)
    {
        if (p[i] != q[i])
        {
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------
// Read from file
//---------------------------------------------------------

TEST_F(Read, SkipComments)
{
    file = {
        "# keyA=1 - commented key",
        "; keyB=1 - commented key",
        " # keyA=1 - commented key",
        " ; keyB=1 - commented key",
        "",
        "key=true"
    };
    load();

    ASSERT_EQ(true, config["key"].value<bool>());
    ASSERT_EQ(1, config.count());
}

TEST_F(Read, KeyWithIndent)
{
    file = {
        "    key1=1",
        "\tkey2=2",
        " \tkey3=3"
    };
    load();

    ASSERT_EQ(1, config["key1"].value<int>());
    ASSERT_EQ(2, config["key2"].value<int>());
    ASSERT_EQ(3, config["key3"].value<int>());
    ASSERT_EQ(3, config.count());
}

TEST_F(Read, EmptyKey)
{
    file = {
        "key="
    };
    load();

    ASSERT_TRUE(config["key"].empty());
}

TEST_F(Read, Bool_True)
{
    file = {
        "key=true"
    };
    load();

    ASSERT_EQ(true, config["key"].value<bool>());
}

TEST_F(Read, Bool_False)
{
    file = {
        "key=false"
    };
    load();

    ASSERT_EQ(false, config["key"].value<bool>());
}

TEST_F(Read, Bool_InvalidValue)
{
    file = {
        "key=10"
    };
    load();

    ASSERT_EQ(false, config["key"].value<bool>());
}

TEST_F(Read, String_Empty)
{
    file = {
        R"(key="")"
    };
    load();

    ASSERT_FALSE(config["key"].empty());
    ASSERT_EQ(std::string{}, config["key"].value<std::string>());
}

TEST_F(Read, ArrayOfEmptyStrings)
{
    file = {
        R"(key=["","","",""])"
    };
    load();

    ASSERT_EQ(4, config["key"].array<int>().size());
}

//---------------------------------------------------------
// Write and read
//---------------------------------------------------------

TEST_F(ReadWrite, Bool_True)
{
    config["key"] = true;
    config["section"]["key"] = true;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(true, output["key"].value<bool>());
    ASSERT_EQ(true, output["section"]["key"].value<bool>());
}

TEST_F(ReadWrite, Bool_False)
{
    config["key"] = false;
    config["section"]["key"] = false;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(false, output["key"].value<bool>());
    ASSERT_EQ(false, output["section"]["key"].value<bool>());
}

TEST_F(ReadWrite, Char_FullRange)
{
    for (auto i = static_cast<int>(std::numeric_limits<char>::min()); i <= static_cast<int>(std::numeric_limits<char>::max()); ++i)
    {
        config["key"] = static_cast<char>(i);
        config["section"]["key"] = static_cast<char>(i);
        writeAndRead(simpleini::SaveFlag_Default);
        ASSERT_EQ(static_cast<char>(i), output["key"].value<char>());
        ASSERT_EQ(static_cast<char>(i), output["section"]["key"].value<char>());
    }
}

TEST_F(ReadWrite, UChar_FullRange)
{
    for (auto i = static_cast<unsigned int>(std::numeric_limits<unsigned char>::min()); i <= static_cast<unsigned int>(std::numeric_limits<unsigned char>::max()); ++i)
    {
        config["key"] = static_cast<unsigned char>(i);
        config["section"]["key"] = static_cast<unsigned char>(i);
        writeAndRead(simpleini::SaveFlag_Default);
        ASSERT_EQ(static_cast<unsigned char>(i), output["key"].value<unsigned char>());
        ASSERT_EQ(static_cast<unsigned char>(i), output["section"]["key"].value<unsigned char>());
    }
}

TEST_F(ReadWrite, Short_Minimum)
{
    config["key"] = std::numeric_limits<short>::min();
    config["section"]["key"] = std::numeric_limits<short>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<short>::min(), output["key"].value<short>());
    ASSERT_EQ(std::numeric_limits<short>::min(), output["section"]["key"].value<short>());
}

TEST_F(ReadWrite, Short_Maximum)
{
    config["key"] = std::numeric_limits<short>::max();
    config["section"]["key"] = std::numeric_limits<short>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<short>::max(), output["key"].value<short>());
    ASSERT_EQ(std::numeric_limits<short>::max(), output["section"]["key"].value<short>());
}

TEST_F(ReadWrite, UShort_Minimum)
{
    config["key"] = std::numeric_limits<unsigned short>::min();
    config["section"]["key"] = std::numeric_limits<unsigned short>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<unsigned short>::min(), output["key"].value<unsigned short>());
    ASSERT_EQ(std::numeric_limits<unsigned short>::min(), output["section"]["key"].value<unsigned short>());
}

TEST_F(ReadWrite, UShort_Maximum)
{
    config["key"] = std::numeric_limits<unsigned short>::max();
    config["section"]["key"] = std::numeric_limits<unsigned short>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<unsigned short>::max(), output["key"].value<unsigned short>());
    ASSERT_EQ(std::numeric_limits<unsigned short>::max(), output["section"]["key"].value<unsigned short>());
}

TEST_F(ReadWrite, Int_Minimum)
{
    config["key"] = std::numeric_limits<int>::min();
    config["section"]["key"] = std::numeric_limits<int>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<int>::min(), output["key"].value<int>());
    ASSERT_EQ(std::numeric_limits<int>::min(), output["section"]["key"].value<int>());
}

TEST_F(ReadWrite, Int_Maximum)
{
    config["key"] = std::numeric_limits<int>::max();
    config["section"]["key"] = std::numeric_limits<int>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<int>::max(), output["key"].value<int>());
    ASSERT_EQ(std::numeric_limits<int>::max(), output["section"]["key"].value<int>());
}

TEST_F(ReadWrite, UInt_Minimum)
{
    config["key"] = std::numeric_limits<unsigned int>::min();
    config["section"]["key"] = std::numeric_limits<unsigned int>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<unsigned int>::min(), output["key"].value<unsigned int>());
    ASSERT_EQ(std::numeric_limits<unsigned int>::min(), output["section"]["key"].value<unsigned int>());
}

TEST_F(ReadWrite, UInt_Maximum)
{
    config["key"] = std::numeric_limits<unsigned int>::max();
    config["section"]["key"] = std::numeric_limits<unsigned int>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<unsigned int>::max(), output["key"].value<unsigned int>());
    ASSERT_EQ(std::numeric_limits<unsigned int>::max(), output["section"]["key"].value<unsigned int>());
}

TEST_F(ReadWrite, Long_Minimum)
{
    config["key"] = std::numeric_limits<long>::min();
    config["section"]["key"] = std::numeric_limits<long>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<long>::min(), output["key"].value<long>());
    ASSERT_EQ(std::numeric_limits<long>::min(), output["section"]["key"].value<long>());
}

TEST_F(ReadWrite, Long_Maximum)
{
    config["key"] = std::numeric_limits<long>::max();
    config["section"]["key"] = std::numeric_limits<long>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<long>::max(), output["key"].value<long>());
    ASSERT_EQ(std::numeric_limits<long>::max(), output["section"]["key"].value<long>());
}

TEST_F(ReadWrite, ULong_Minimum)
{
    config["key"] = std::numeric_limits<unsigned long>::min();
    config["section"]["key"] = std::numeric_limits<unsigned long>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<unsigned long>::min(), output["key"].value<unsigned long>());
    ASSERT_EQ(std::numeric_limits<unsigned long>::min(), output["section"]["key"].value<unsigned long>());
}

TEST_F(ReadWrite, ULong_Maximum)
{
    config["key"] = std::numeric_limits<unsigned long>::max();
    config["section"]["key"] = std::numeric_limits<unsigned long>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<unsigned long>::max(), output["key"].value<unsigned long>());
    ASSERT_EQ(std::numeric_limits<unsigned long>::max(), output["section"]["key"].value<unsigned long>());
}

TEST_F(ReadWrite, LongLong_Minimum)
{
    config["key"] = std::numeric_limits<long long>::min();
    config["section"]["key"] = std::numeric_limits<long long>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<long long>::min(), output["key"].value<long long>());
    ASSERT_EQ(std::numeric_limits<long long>::min(), output["section"]["key"].value<long long>());
}

TEST_F(ReadWrite, LongLong_Maximum)
{
    config["key"] = std::numeric_limits<long long>::max();
    config["section"]["key"] = std::numeric_limits<long long>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<long long>::max(), output["key"].value<long long>());
    ASSERT_EQ(std::numeric_limits<long long>::max(), output["section"]["key"].value<long long>());
}

TEST_F(ReadWrite, ULongLong_Minimum)
{
    config["key"] = std::numeric_limits<unsigned long long>::min();
    config["section"]["key"] = std::numeric_limits<unsigned long long>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<unsigned long long>::min(), output["key"].value<unsigned long long>());
    ASSERT_EQ(std::numeric_limits<unsigned long long>::min(), output["section"]["key"].value<unsigned long long>());
}

TEST_F(ReadWrite, ULongLong_Maximum)
{
    config["key"] = std::numeric_limits<unsigned long long>::max();
    config["section"]["key"] = std::numeric_limits<unsigned long long>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<unsigned long long>::max(), output["key"].value<unsigned long long>());
    ASSERT_EQ(std::numeric_limits<unsigned long long>::max(), output["section"]["key"].value<unsigned long long>());
}

TEST_F(ReadWrite, Float_Minimum)
{
    config["key"] = std::numeric_limits<float>::min();
    config["section"]["key"] = std::numeric_limits<float>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<float>::min(), output["key"].value<float>());
    ASSERT_EQ(std::numeric_limits<float>::min(), output["section"]["key"].value<float>());
}

TEST_F(ReadWrite, Float_Maximum)
{
    config["key"] = std::numeric_limits<float>::max();
    config["section"]["key"] = std::numeric_limits<float>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<float>::max(), output["key"].value<float>());
    ASSERT_EQ(std::numeric_limits<float>::max(), output["section"]["key"].value<float>());
}

TEST_F(ReadWrite, Double_Minimum)
{
    config["key"] = std::numeric_limits<double>::min();
    config["section"]["key"] = std::numeric_limits<double>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<double>::min(), output["key"].value<double>());
    ASSERT_EQ(std::numeric_limits<double>::min(), output["section"]["key"].value<double>());
}

TEST_F(ReadWrite, Double_Maximum)
{
    config["key"] = std::numeric_limits<double>::max();
    config["section"]["key"] = std::numeric_limits<double>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<double>::max(), output["key"].value<double>());
    ASSERT_EQ(std::numeric_limits<double>::max(), output["section"]["key"].value<double>());
}

TEST_F(ReadWrite, LongDouble_Minimum)
{
    config["key"] = std::numeric_limits<long double>::min();
    config["section"]["key"] = std::numeric_limits<long double>::min();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<long double>::min(), output["key"].value<long double>());
    ASSERT_EQ(std::numeric_limits<long double>::min(), output["section"]["key"].value<long double>());
}

TEST_F(ReadWrite, LongDouble_Maximum)
{
    config["key"] = std::numeric_limits<long double>::max();
    config["section"]["key"] = std::numeric_limits<long double>::max();
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(std::numeric_limits<long double>::max(), output["key"].value<long double>());
    ASSERT_EQ(std::numeric_limits<long double>::max(), output["section"]["key"].value<long double>());
}


TEST_F(ReadWrite, String_Empty)
{
    std::string text {};

    config["key"] = text;
    config["section"]["key"] = text;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(text, output["key"].value<std::string>());
    ASSERT_EQ(text, output["section"]["key"].value<std::string>());
}

TEST_F(ReadWrite, String_TextWithoutSpaces)
{
    std::string text {"thisisstring"};

    config["key"] = text;
    config["section"]["key"] = text;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(text, output["key"].value<std::string>());
    ASSERT_EQ(text, output["section"]["key"].value<std::string>());
}

TEST_F(ReadWrite, String_TextWithSpaces)
{
    std::string text {"this is string"};

    config["key"] = text;
    config["section"]["key"] = text;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(text, output["key"].value<std::string>());
    ASSERT_EQ(text, output["section"]["key"].value<std::string>());
}

TEST_F(ReadWrite, String_Multiline)
{
    std::string text {"this\nis\nstring"};

    config["key"] = text;
    config["section"]["key"] = text;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(text, output["key"].value<std::string>());
    ASSERT_EQ(text, output["section"]["key"].value<std::string>());
}

TEST_F(ReadWrite, String_SpecialCharacters)
{
    std::string text {"'\"\t\n\r"};

    config["key"] = text;
    config["section"]["key"] = text;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_EQ(text, output["key"].value<std::string>());
    ASSERT_EQ(text, output["section"]["key"].value<std::string>());
}

TEST_F(ReadWrite, Vector_Int)
{
    std::vector<int> vec { {1, 2, 3} };

    config["key"] = vec;
    config["section"]["key"] = vec;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_TRUE(compareArrays(vec, output["key"].array<int>()));
    ASSERT_TRUE(compareArrays(vec, output["section"]["key"].array<int>()));
}

TEST_F(ReadWrite, Vector_Float)
{
    std::vector<float> vec { {1.4f, 2.6f, 3.8f} };

    config["key"] = vec;
    config["section"]["key"] = vec;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_TRUE(compareArrays(vec, output["key"].array<float>()));
    ASSERT_TRUE(compareArrays(vec, output["section"]["key"].array<float>()));
}

TEST_F(ReadWrite, Vector_String)
{
    std::vector<std::string> vec { {"text", "", "\"a b\tc\"\r\n"} };

    config["key"] = vec;
    config["section"]["key"] = vec;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_TRUE(compareArrays(vec, output["key"].array<std::string>()));
    ASSERT_TRUE(compareArrays(vec, output["section"]["key"].array<std::string>()));
}

TEST_F(ReadWrite, Vector_Empty)
{
    std::vector<std::string> vec {};

    config["key"] = vec;
    config["section"]["key"] = vec;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_TRUE(compareArrays(vec, output["key"].array<std::string>()));
    ASSERT_TRUE(compareArrays(vec, output["section"]["key"].array<std::string>()));
}

TEST_F(ReadWrite, ListAsArray)
{
    std::list<std::string> list {{"1", "2", "3"}};
    std::vector<std::string> out {{"1", "2", "3"}};

    config["key"] = list;
    config["section"]["key"] = list;
    writeAndRead(simpleini::SaveFlag_Default);
    ASSERT_TRUE(compareArrays(out, output["key"].array<std::string>()));
    ASSERT_TRUE(compareArrays(out, output["section"]["key"].array<std::string>()));
}
