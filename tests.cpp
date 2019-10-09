#include <gtest/gtest.h>
#include "parser.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <random>

using namespace std;
using namespace testing;

const time_t T_STAMP{1570289783};

class IParserTestSuite
{
    virtual void writeInData(size_t) = 0;
};

struct GenericParserTestSuite : public IParserTestSuite, public Test
{
    void generateInputFile(size_t size = 0)
    {
        if(createFile())
        {
            writeInData(size);
            outFile.close();
        };
    };
    std::string fileName{"herpDerp.dat"};
    bool createFile()
    {
        outFile = ofstream(fileName, ios::out);
        return outFile.is_open();
    };
    void writeInData(size_t size) override
    {
        outFile << T_STAMP    << ",s1,f1,6,f2,9,f3,12,f4,10" << endl;
        outFile << T_STAMP+10 << ",s1,f1,7,f2,10,f3,13,f4,11" << endl;
        outFile << T_STAMP+20 << ",s2,f1,5,f2,8,f3,11,f4,9" << endl;
    };
    ofstream outFile;
    void SetUp()
    {
        generateInputFile();
    }
    virtual void TearDown()
    {
        remove(fileName.c_str());
    }
};

TEST_F(GenericParserTestSuite, dummy)
{
    ASSERT_TRUE(true);
}

TEST_F(GenericParserTestSuite, createObject)
{
    Parser sut{};
}

TEST_F(GenericParserTestSuite, returnsFalseWhileTryingToOpenNonexistingFile)
{
    Parser sut{};
    ASSERT_FALSE(sut.openTickFile("herpderp"));
}

TEST_F(GenericParserTestSuite, returnsTrueWhilstOpeningEmptyFile)
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

struct SmallFileParserProductParamTestSuite : GenericParserTestSuite,
                                              WithParamInterface<ProductParam>
{};

INSTANTIATE_TEST_CASE_P(ProductTest, SmallFileParserProductParamTestSuite,
                       Values(ProductParam{T_STAMP-1, T_STAMP+1, "s1", "f1", "f2", "54.000"},
                              ProductParam{T_STAMP-1, T_STAMP+1, "s1", "f2", "f3", "108.000"},
                              ProductParam{T_STAMP-1, T_STAMP+1, "s1", "f1", "f4", "60.000"},
                              ProductParam{T_STAMP-1, T_STAMP+1, "s2", "f1", "f4", ""},
                              ProductParam{T_STAMP+1, T_STAMP+2, "s1", "f1", "f2", ""},
                              ProductParam{T_STAMP-1, T_STAMP+11,"s1", "f1", "f2", "124.000"},
                              ProductParam{T_STAMP-1, T_STAMP+21,"s1", "f1", "f2", "124.000"}));

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
    string  expectation;
};

struct SmallFileParserPrintParamTestSuite : GenericParserTestSuite,
                                            WithParamInterface<PrintParam>
{};

INSTANTIATE_TEST_CASE_P(PrintTest, SmallFileParserPrintParamTestSuite,
                        Values(PrintParam{T_STAMP-1, T_STAMP+1, "s1", "f1:6.000,f2:9.000,f3:12.000,f4:10.000\n"},
                               PrintParam{T_STAMP-1, T_STAMP+1, "s2", ""},
                               PrintParam{T_STAMP+1, T_STAMP+2, "s1", ""},
                               PrintParam{T_STAMP-1, T_STAMP+11,"s1", "f1:6.000,f2:9.000,f3:12.000,f4:10.000\nf1:7.000,f2:10.000,f3:13.000,f4:11.000\n"},
                               PrintParam{T_STAMP-1, T_STAMP+21,"s1", "f1:6.000,f2:9.000,f3:12.000,f4:10.000\nf1:7.000,f2:10.000,f3:13.000,f4:11.000\n"}));

TEST_P(SmallFileParserPrintParamTestSuite, expectSpecificPrint)
{
    Parser sut{};
    sut.openTickFile("herpDerp.dat");
    auto param = GetParam();
    testing::internal::CaptureStdout();
    sut.print(param.rangeStart, param.rangeEnd, param.symbol);
    EXPECT_EQ(param.expectation, testing::internal::GetCapturedStdout());
}

struct PerfParam
{
    size_t fileSize;        //in bytes
    time_t rangeStart;
    time_t rangeEnd;
    string symbol;
    string field1;
    string field2;
};

struct LargeInputFileParserParametricPerformanceTestSuite : GenericParserTestSuite,
                                                            WithParamInterface<PerfParam>
{
    virtual void writeInData(size_t byteCount) override
    {
        const vector<string> symbolTable = {"s0","s1","s2","s3","s4","s5","s6","s7","s8","s9"};
        const vector<string> fieldTable = {"f0","f1","f2","f3","f4","f5","f6","f7","f8","f9"};
        random_device rDev;
        default_random_engine randIntEngine(rDev());
        mt19937 randValueEngine(rDev());
        uniform_int_distribution<unsigned> randTime(0, 10);                      //time increments
        uniform_int_distribution<unsigned> randSymbol(0, 9);
        uniform_int_distribution<unsigned> randField(1, 1024);                   //the binary rep determines which field is present from the field table

        normal_distribution<> randValue(15.0, 5.0);

        unsigned time=T_STAMP;

        auto generateLine = [&]() -> stringstream
        {
            stringstream out{};
            time += randTime(randIntEngine);
            out << time << "," << symbolTable[randSymbol(randIntEngine)];
            for(auto i = randField(randIntEngine), bit = 0u; i > 0; i >>= 1, ++bit)
            {
                if(i & 1)
                {
                    out << "," << fieldTable[bit] << "," << std::fixed << std::setprecision(3) << randValue(randValueEngine);
                }

            }
            out << "\n";
            return out;
        };

        while((int)byteCount > 0)
        {
            time += randTime(randIntEngine);
            auto line = generateLine();
            outFile << line.str();
            byteCount -= line.str().size();
        }
    }

    virtual void TearDown() override
    {
        //remove(fileName.c_str());
    }

};

INSTANTIATE_TEST_CASE_P(PerfTest, LargeInputFileParserParametricPerformanceTestSuite,
                        Values(PerfParam{1024, T_STAMP-1, T_STAMP+1, "s1", "", ""},
                               PerfParam{1024*1024, T_STAMP-1, T_STAMP+1, "s1", "", ""},
                               PerfParam{1024*1024*512, T_STAMP+1000, T_STAMP+100000, "s1", "", ""}));

TEST_P(LargeInputFileParserParametricPerformanceTestSuite, performLargePrintsTests)
{
    fileName = "perf.dat";
    auto param = GetParam();
    generateInputFile(param.fileSize);

    Parser sut{};
    ASSERT_TRUE(sut.openTickFile("perf.dat"));
    sut.print(param.rangeStart, param.rangeEnd, param.symbol);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
