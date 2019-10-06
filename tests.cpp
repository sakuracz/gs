#include <gtest/gtest.h>
#include "parser.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>

using namespace std;
using namespace testing;

const time_t T_STAMP{1570289783};

struct SmallFileParserTestSuite : public Test
{
    void generateInputFile()
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
        return outFile.is_open();
    };
    void writeInData()
    {
        outFile << T_STAMP    << ",s1,f1,6,f2,9,f3,12,f4,10" << endl;
        outFile << T_STAMP+10 << ",s1,f1,7,f2,10,f3,13,f4,11" << endl;
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

struct ProductParam
{
    time_t rangeStart;
    time_t rangeEnd;
    string symbol;
    string field1;
    string field2;
    string expectation;
};

struct SmallFileParserProductParamTestSuite : SmallFileParserTestSuite,
                                              WithParamInterface<ProductParam>
{};

INSTANTIATE_TEST_CASE_P(ProductTest, SmallFileParserProductParamTestSuite,
                       Values(ProductParam{T_STAMP-1, T_STAMP+1, "s1", "f1", "f2", "54.000"},
                              ProductParam{T_STAMP-1, T_STAMP+1, "s1", "f2", "f3", "108.000"},
                              ProductParam{T_STAMP-1, T_STAMP+1, "s1", "f1", "f4", "60.000"},
                              ProductParam{T_STAMP-1, T_STAMP+1, "s2", "f1", "f4", ""},
                              ProductParam{T_STAMP+1, T_STAMP+2, "s1", "f1", "f2", ""},
                              ProductParam{T_STAMP-1, T_STAMP+11,"s1", "f1", "f2", "124.000"}));

TEST_P(SmallFileParserProductParamTestSuite, expectSpecificProduct)
{
    Parser sut{};
    sut.openTickFile("herpDerp.dat");
    auto param = GetParam();
    testing::internal::CaptureStdout();
    sut.product(param.rangeStart, param.rangeEnd, param.symbol, param.field1, param.field2);
    EXPECT_EQ(param.expectation, testing::internal::GetCapturedStdout());
}

struct PrintParam
{
    time_t rangeStart;
    time_t rangeEnd;
    string symbol;
    string expectation;
};

struct SmallFileParserPrintParamTestSuite : SmallFileParserTestSuite,
                                            WithParamInterface<PrintParam>
{};

INSTANTIATE_TEST_CASE_P(PrintTest, SmallFileParserPrintParamTestSuite,
                        Values(PrintParam{T_STAMP-1, T_STAMP+1, "s1", "f1:6.000,f2:9.000,f3:12.000,f4:10.000\n"},
                               PrintParam{T_STAMP-1, T_STAMP+1, "s2", ""},
                               PrintParam{T_STAMP+1, T_STAMP+2, "s1", ""},
                               PrintParam{T_STAMP-1, T_STAMP+11,"s1", "f1:6.000,f2:9.000,f3:12.000,f4:10.000\nf1:7.000,f2:10.000,f3:13.000,f4:11.000\n"}));

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
