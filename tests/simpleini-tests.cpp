#include <limits>

#include "gtest/gtest.h"
#include "simpleini.h"

//---------------------------------------------------------
// Key
//---------------------------------------------------------

TEST(Value, Empty)
{
    simpleini::Config config;
    ASSERT_EQ(true, config["key"].empty());
    ASSERT_EQ(true, config["section"]["key"].empty());
}

TEST(Key, NotEmpty)
{
    {
        simpleini::Config config;
        config["key"] = 1;
        config["section"]["key"] = 1;
        ASSERT_EQ(false, config["key"].empty());
        ASSERT_EQ(false, config["section"]["key"].empty());
    }
    {
        simpleini::Config config;
        config["key"] = std::string{};
        config["section"]["key"] = std::string{""};
        ASSERT_EQ(false, config["key"].empty());
        ASSERT_EQ(false, config["section"]["key"].empty());
    }
}

TEST(Key, Clear)
{
    simpleini::Config config;
    config["key"] = 1;
    config["section"]["key"] = 1;
    config["key"].clear();
    config["section"]["key"].clear();
    ASSERT_EQ(true, config["key"].empty());
    ASSERT_EQ(true, config["section"]["key"].empty());
}

TEST(Key, DefaultValue)
{
    simpleini::Value value;
    ASSERT_EQ(-1, value.value<int>(-1));
}

//---------------------------------------------------------
// Section
//---------------------------------------------------------

TEST(Section, AddKeys)
{
    simpleini::Config config;
    config["key_1"] = 1;
    config["key_2"] = 2;
    config["key_3"] = 3;
    config["section"]["key_1"] = 10;
    config["section"]["key_2"] = 20;
    config["section"]["key_3"] = 30;
    ASSERT_EQ(1, config["key_1"].value<int>());
    ASSERT_EQ(2, config["key_2"].value<int>());
    ASSERT_EQ(3, config["key_3"].value<int>());
    ASSERT_EQ(10, config["section"]["key_1"].value<int>());
    ASSERT_EQ(20, config["section"]["key_2"].value<int>());
    ASSERT_EQ(30, config["section"]["key_3"].value<int>());
}

TEST(Section, Clear)
{
    simpleini::Config config;
    config["section"]["key_1"] = 10;
    config["section"]["key_2"] = 20;
    config["section"]["key_3"] = 30;
    config["section"].clear();
    ASSERT_EQ(true, config["section"].empty());
}

TEST(Section, IsSection)
{
    simpleini::Config config;
    config["section"]["key_1"] = 10;
    ASSERT_EQ(true, config["section"].section());
}

TEST(Section, IsNotSection)
{
    simpleini::Config config;
    config["not_section"] = 10;
    ASSERT_EQ(false, config["not_section"].section());
}

//---------------------------------------------------------
// Config
//---------------------------------------------------------

TEST(Config, Count)
{
    simpleini::Config config;
    config["key1"] = 1;
    config["key2"] = 2;
    config["section_1"]["key1"] = 11;
    config["section_1"]["key2"] = 12;
    config["section_2"]["key1"] = 21;
    config["section_2"]["key2"] = 22;
    ASSERT_EQ(6, config.count());
}

TEST(Config, CountEmpty)
{
    simpleini::Config config;
    ASSERT_EQ(0, config.count());
}
