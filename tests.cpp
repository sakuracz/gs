#include <gtest/gtest.h>
#include "parser.hpp"

using namespace testing;

class parserTestSuite : public Test
{
};

TEST_F(parserTestSuite, dummy)
{
    ASSERT_TRUE(true);
}

TEST_F(parserTestSuite, createObject)
{
    parser sut{};
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
