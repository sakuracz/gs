#include <gtest/gtest.h>
#include "parser.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>

using namespace std;
using namespace testing;

const time_t T_STAMP{1570289783};

struct IParserTestSuite
{
    virtual void generateInputFile() = 0;
};

struct SmallFileParserTestSuite : public IParserTestSuite, public Test
{
    void generateInputFile() override
    {
        if(createFile())
        {
            writeInData();
            outFile.close();
        };
    };
    std::string fileName{"herpDerp.dat"};
    bool createFile()
    {
        outFile = ofstream(fileName, ios::out);
        outFile.is_open();
    };
    void writeInData()
    {
        outFile << T_STAMP << ",s1,f1,6,f2,9,f3,12,f4,10" << endl;
    };
    ofstream outFile;
    void SetUp()
    {
        generateInputFile();
    }
    void TearDown()
    {
        remove(fileName.c_str());
    }
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
    ASSERT_TRUE(sut.openTickFile("herpDerp.dat"));
}

struct TestParam
{
    time_t rangeStart;
    time_t rangeEnd;
    string symbol;
    string field1;
    string field2;
    const string expectation;
};

struct SmallFileParserProductParamTestSuite : SmallFileParserTestSuite,
                                              WithParamInterface<TestParam>
{};

INSTANTIATE_TEST_CASE_P(ProductTest, SmallFileParserProductParamTestSuite,
                       Values(TestParam{T_STAMP-1, T_STAMP+1, "s1", "f1", "f2", "54.000"},
                              TestParam{T_STAMP-1, T_STAMP+1, "s1", "f2", "f3", "72.000"},
                              TestParam{T_STAMP-1, T_STAMP+1, "s1", "f1", "f4", "60.000"},
                              TestParam{T_STAMP-1, T_STAMP+1, "s2", "f1", "f4", ""},
                              TestParam{T_STAMP+1, T_STAMP+2, "s1", "f1", "f2", ""}));

TEST_P(SmallFileParserProductParamTestSuite, expectSpecificProduct)
{
    Parser sut{};
    sut.openTickFile("herpDerp.dat");
    auto param = GetParam();
    testing::internal::CaptureStdout();
    sut.product(param.rangeStart, param.rangeEnd, param.symbol, param.field1, param.field2);
    EXPECT_EQ(param.expectation, testing::internal::GetCapturedStdout());
}

struct SmallFileParserPrintParamTestSuite : SmallFileParserTestSuite,
                                              WithParamInterface<TestParam>
{};

INSTANTIATE_TEST_CASE_P(PrintTest, SmallFileParserPrintParamTestSuite,
                        Values(TestParam{T_STAMP-1, T_STAMP+1, "s1", "", "", "f1:6.000,f2:9.000,f3:12.000,f4:10.000"},
                               TestParam{T_STAMP-1, T_STAMP+1, "s2", "", "", ""},
                               TestParam{T_STAMP+1, T_STAMP+2, "s1", "", "", ""}));

TEST_P(SmallFileParserPrintParamTestSuite, expectSpecificPrint)
{
    Parser sut{};
    sut.openTickFile("herpDerp.dat");
    auto param = GetParam();
    testing::internal::CaptureStdout();
    sut.print(param.rangeStart, param.rangeEnd, param.symbol);
    EXPECT_EQ(param.expectation, testing::internal::GetCapturedStdout());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
