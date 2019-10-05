#include <gtest/gtest.h>
#include "parser.hpp"
#include <string>
#include <fstream>

using namespace std;
using namespace testing;

class IParserTestSuite
{
private:
    virtual void generateInputFile() = 0;
};

class SmallFileParserTestSuite : public IParserTestSuite, public Test
{
private:
    void generateInputFile() override
    {
        if(createFile())
        {
            outFile.close();
        };
    };
    std::string fileName{"herpDerp.dat"};
    bool createFile()
    {
        outFile = ofstream(fileName, ios::out);
        outFile.is_open();
    };
    ofstream outFile;
};

TEST_F(SmallFileParserTestSuite, dummy)
{
    ASSERT_TRUE(true);
}

TEST_F(SmallFileParserTestSuite, createObject)
{
    Parser sut{};
}

TEST_F(SmallFileParserTestSuite, returnsFalseWhileTryingToOpenNonexistingFile)
{
    Parser sut{};
    ASSERT_FALSE(sut.openTickFile("herpderp"));
}

TEST_F(SmallFileParserTestSuite, returnsTrueWhilstOpeningEmptyFile)
{
    Parser sut{};
    ASSERT_TRUE(sut.openTickFile("herpderp"));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
