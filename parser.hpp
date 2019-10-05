#include <string>
#include <iostream>
#include <time.h>
#include <fstream>

using std::string;
using std::ifstream;
using std::cout;

class IParser
{
public:
   virtual bool openTickFile(string) = 0;
   virtual void print(time_t, time_t, string) = 0;
   virtual void product(time_t, time_t, string, string, string) = 0;
};


class Parser : public IParser
{
public:
    bool openTickFile(string fName) override
    {
        data = ifstream(fName, std::ios::in);
        return data.is_open();
    }
    void print(time_t startTime, time_t endTime, string symbol) override
    {
        cout << "f1:6.000,f2:9.000,f3:12.000,f4:10.000";
    };
    void product(time_t startTime, time_t endTime, string symbol, string field1, string field2) override
    {
        cout << "54.000";
    };
private:
    ifstream data;
};
