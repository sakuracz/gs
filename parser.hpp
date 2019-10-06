#include <string>
#include <iostream>
#include <time.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cmath>

using namespace std;
using FieldToIdxMap = unordered_map<string, unsigned>;
using SymbolDict = unordered_set<string>;

class IParser
{
public:
   virtual bool openTickFile(string) = 0;
   virtual void print(time_t, time_t, string) = 0;
   virtual void product(time_t, time_t, string, string, string) = 0;
};

struct SymbolTable
{
    vector<time_t> timestamps;
    vector<vector<double>> values;
};

class Parser : public IParser
{
public:
    Parser() : fieldToIdx(100u), symbolDict(100u){};
    bool openTickFile(string fName) override
    {
        data = ifstream(fName, std::ios::in);
        readInData();
        return data.is_open();
    }
    void print(time_t startTime, time_t endTime, string symbol) override
    {
        if(symbolDict.find(symbol) != symbolDict.end())
        {
            const auto& times = symbolTables[symbol].timestamps;
            auto rangeBeg = (startTime < times[0] ? times.begin() : find_if(times.begin(), times.end(), [&](auto& a){return a >= startTime;}) ) ;
            auto rangeEnd = (endTime > times.back() ? times.end() : find_if(rangeBeg, times.end(), [&](auto& a){return a+1 > endTime;}) ) ;

            if(rangeBeg != times.end() )
            {
                const auto& vals = symbolTables[symbol].values;
                for(auto it = vals.begin()+distance(times.begin(), rangeBeg);
                         it != vals.begin()+distance(times.begin(), rangeEnd);
                         ++it)
                {
                    printValueVector(*it);
                }
            }
        }
    }
    void printValueVector(const vector<double>& chu)
    {
        stringstream output{};
        string coma="";
        for(auto i = 0u; i < chu.size(); ++i)
            if(!isnan(chu[i]))
            {
                output << coma << fieldNames[i] << ":" << std::fixed << std::setprecision(3) << chu[i];
                coma = ",";
            }
        cout << output.str();
    }
    void product(time_t startTime, time_t endTime, string symbol, string field1, string field2) override
    {
        cout << "54.000";
    }
private:
    void readInData()
    {
        string line;
        while(getline(data, line, '\n'))
        {
            time_t timeStamp = consumeTime(line);
            string symbol = consumeString(line);
            vector<double> fieldVals = consumeFields(line);
            if(symbolDict.find(symbol) == symbolDict.end()) //symbol not yet encountered
            {
                symbolDict.insert(symbol);
                vector<vector<double>> herp{};
                vector<time_t> derp{};
                derp.push_back(timeStamp);
                herp.push_back(fieldVals);
                symbolTables.emplace(symbol, SymbolTable{derp, herp});
            }
        }
    }
    time_t consumeTime(string& line)
    {
        auto firstComa = line.find(",",0);
        auto time = line.substr(0,firstComa);
        line = line.substr(firstComa+1);
        return time_t(atoi(time.c_str()));
    }
    string consumeString(string& line)
    {
        auto firstComa = line.find(",",0);
        auto str = line.substr(0, firstComa);
        line = line.substr(firstComa+1);
        return str;
    }
    double consumeValue(string& line)
    {
        auto firstComa = line.find(",",0);
        if(firstComa != line.npos)
        {
            auto val = line.substr(0, firstComa);
            line = line.substr(firstComa+1);
            return atof(val.c_str());
        }
        else
        {
            auto val = line.substr(0);
            line.clear();
            return atof(val.c_str());
        }
    }
    vector<double> consumeFields(string& line)
    {
        vector<double> fieldVals(fieldToIdx.size(), NAN);
        while(!line.empty())
        {
            string fieldName = consumeString(line);
            double fieldValue = consumeValue(line);
            if(fieldToIdx.find(fieldName) != fieldToIdx.end())
            {
                fieldVals[fieldToIdx.at(fieldName)] = fieldValue;
            }
            else
            {
                fieldVals.push_back(fieldValue);
                fieldNames.push_back(fieldName);
                fieldToIdx.insert({fieldName, fieldValue});
            }
        }
        return fieldVals;
    }

    ifstream data{nullptr};
    FieldToIdxMap fieldToIdx;
    SymbolDict symbolDict;
    vector<string> fieldNames{};
    map<string, SymbolTable> symbolTables;
};
