#include <gtest/gtest.h>
#include "parser.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <random>
#include <chrono>

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
                       Values(ProductParam{T_STAMP-1, T_STAMP+1, "s1", "f1", "f2", "54.000\n"},
                              ProductParam{T_STAMP-1, T_STAMP+1, "s1", "f2", "f3", "108.000\n"},
                              ProductParam{T_STAMP-1, T_STAMP+1, "s1", "f1", "f4", "60.000\n"},
                              ProductParam{T_STAMP-1, T_STAMP+1, "s2", "f1", "f4", ""},
                              ProductParam{T_STAMP+1, T_STAMP+2, "s1", "f1", "f2", ""},
                              ProductParam{T_STAMP-1, T_STAMP+11,"s1", "f1", "f2", "124.000\n"},
                              ProductParam{T_STAMP-1, T_STAMP+21,"s1", "f1", "f2", "124.000\n"}));

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

struct LargeFileWriterTestSuite : GenericParserTestSuite
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
        uniform_int_distribution<unsigned> randField(1, 1023);                   //the binary rep determines which field is present from the field table

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
};

struct LargeInputFileParserParametricPerformanceTestSuite : LargeFileWriterTestSuite,
                                                            WithParamInterface<PerfParam>
{
    LargeInputFileParserParametricPerformanceTestSuite()
    {
        auto now = chrono::system_clock::now();
        auto in_time_t = chrono::system_clock::to_time_t(now);

        stringstream today;
        today << put_time(localtime(&in_time_t), "%Y-%m-%d %X");
        static bool isFileCreated = false;
        if(not isFileCreated)
        {
            outplot.open("Fig1.eps", ofstream::out | ofstream::trunc);
            if(outplot)
            {
                outplot << "%!PS-Adobe-3.0" << endl << "%%BoundingBox: 0 0 595 842" << endl
                        << "%%Copyright: (Michal 'DRINKer' Kozub)" << endl
                        << "%%Creator: (parser.cpp)" << endl << "%%CreationDate: ("
                        << today.str() << ")" << endl << "%%DocumentData: Clean7Bit" << endl
                        << "%%Pages: 1" << endl << "%%For: the watch" << endl
                        << "%%LanguageLevel: 1" << endl << "%%Orientation: landscape" << endl
                        << "%%PageOrder: Ascend" << endl << "%%Title: (Fig.1 Time vs Set size)" << endl
                        << "%%Version: 1.0" << endl << "%%DocumentNeededResources: font Times-Roman" << endl
                        << "%%DocumentMedia: A4 842 1190 72 white ( )" << endl
                        << "%%DocumentFonts: Times-Roman" << endl
                        << "%%DocumentSuppliedResources: showpowa strhght invscale centrestr vertext extrcoords xtick ytick" << endl
                        << "%%EndComments" << endl
                        << "%%BeginProlog" << endl << "%%BeginResource: procset" << endl
                        << "/showpowa % stk: string" << endl
                        << "{0.04 1.02 moveto (x10) centrestr 0.01 0.015 rmoveto 0.8 0.8 scale centrestr 0.8 0.8 invscale} def " << endl
                        << "%%EndResource" << endl << "%%BeginResource: procset" << endl
                        << "/invscale % stk : x y" << endl
                        << "{1 exch div exch 1 exch div exch scale} def" << endl
                        << "%%EndResource" << endl << "%%BeginResource: procset" << endl
                        << "/centrestr % stk : string" << endl
                        << "{dup stringwidth pop dup 2 div exch sub 0 rmoveto show} def" << endl
                        << "%%EndResource" << endl << "%%BeginResource: procset" << endl
                        << "/singularBox {newpath 0 0 moveto 0 1 lineto 1 1 lineto 1 0 lineto closepath} def" << endl
                        << "%%EndResource" << endl << "%%BeginResource: procset" << endl
                        << "/vertex % stk : string x y" << endl
                        << "{moveto gsave currentpoint translate 90 rotate centrestr grestore} def" << endl
                        << "%%EndResource" << endl << "%%BeginResource: procset" << endl
                        << "/extrcoords {dup coords exch get 1 add coords exch get} def" << endl
                        <<  "%%EndResource" << endl << "%%BeginResource: procset" << endl
                        << "/xtick % stk : x" << endl
                        << "{0 moveto 0 -0.02 rlineto -0.02 0 rmoveto stroke} def" << endl
                        << "%%EndResource" << endl << "%%BeginResource: procset" << endl
                        << "/ytick % stk : x y string" << endl
                        << "{0 exch moveto -0.02 0 rlineto -0.02 0 rmoveto stroke} def" << endl
                        << "%%EndResource" << endl << "%%BeginResource: procset" << endl
                        << "/strhghtrel % stk : x" << endl << "{1 exch div scalefont} def" << endl
                        << "%%EndResource" << endl << "%%EndProlog" << endl
                        << "%%BeginSetup" << endl << "/Times-Roman findfont" << endl
                        << "10 scalefont" << endl << "setfont" << endl << "0.0 0.0 0.0 setrgbcolor" << endl
                        << "%%EndSetup" << endl;
                //EOF file header
                outplot << "%%Page: Page1 1" << endl << "%%BeginPageSetup" << endl << "%%EndPageSetup" << endl
                        << "1 1 scale" << endl << "0.0 0.0 0.0 setrgbcolor" << endl
                        << "0.005 setlinewidth" << endl
                        << "newpath" << endl << "singularBox stroke" << endl << "" << endl
                        << "showpage" << endl << "%%PageTrailer" << endl << "%%Trailer" << endl
                        << "%%EOF" << endl;
                isFileCreated = true;
            }
        }
    }
    virtual void TearDown() override
    {
        //remove(fileName.c_str());
    }

    ofstream outplot{nullptr};


};

INSTANTIATE_TEST_CASE_P(PerfTest, LargeInputFileParserParametricPerformanceTestSuite,
                        Values(PerfParam{1024,          T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"},     //  1 kiB
                               PerfParam{1024*4,        T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"},     //  4 kiB
                               PerfParam{1024*16,       T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"},     // 16 kiB
                               PerfParam{1024*64,       T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"},     // 64 kiB
                               PerfParam{1024*256,      T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"},     //256 kiB
                               PerfParam{1024*1024,     T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"},     //  1 MiB
                               PerfParam{1024*1024*4,   T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"},     //  4 MiB
                               PerfParam{1024*1024*16,  T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"},     // 16 MiB
                               PerfParam{1024*1024*64,  T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"},     // 64 MiB
                               PerfParam{1024*1024*256, T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"},     //256 MiB
                               PerfParam{1024*1024*512, T_STAMP+10, T_STAMP*2, "s1", "f0", "f1"}));   //512 MiB

TEST_P(LargeInputFileParserParametricPerformanceTestSuite, performLargePrintsTests)
{
    fileName = "perf.dat";
    auto param = GetParam();
    generateInputFile(param.fileSize);

    Parser sut{};
    auto readStart = std::chrono::system_clock::now();
    ASSERT_TRUE(sut.openTickFile("perf.dat"));
    auto readEnd = std::chrono::system_clock::now();
    auto readTime = std::chrono::duration_cast<std::chrono::nanoseconds>(readEnd - readStart);

    auto printStart = std::chrono::system_clock::now();
    sut.print(param.rangeStart, param.rangeEnd, param.symbol);
    auto printEnd = std::chrono::system_clock::now();
    auto printTime = std::chrono::duration_cast<std::chrono::nanoseconds>(printEnd - printStart);

    auto productStart = std::chrono::system_clock::now();
    sut.product(param.rangeStart, param.rangeEnd, param.symbol, param.field1, param.field2);
    auto productEnd = std::chrono::system_clock::now();
    auto productTime = std::chrono::duration_cast<std::chrono::nanoseconds>(productEnd - productStart);

    cerr << param.fileSize << "\t" << readTime.count() << "\t" << printTime.count() << "\t" << productTime.count() << endl;
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
