#include <memory>
#include "parser.hpp"

using namespace std;

namespace
{
template<typename T>
unique_ptr<IParser> parserFactory()
{
    return make_unique<T>();
}
} //eof anon namespace

int main(int argc, char **argv)
{
    unique_ptr<IParser> parser{nullptr};

    if(argc == 1)       //no optimization //boost::program_options would handle this more gracefuly
    {
        parser = parserFactory<Parser>();
    }
    else if(argc > 2){
        cout << "Invalid parameter count \nTerminating." << endl;
        return 0;
    }
    else
    {
        string param = argv[1];
        if(param == "-Oprint")
        {
            parser = parserFactory<PrintParser>();              //TODO: actually implement this
        }
        else if(param == "-Oproduct")
        {
            parser = parserFactory<ProductParser>();            //TODO: actually implement this
        }
        else
        {
            cout << "Unrecognized parameter. Terminating.";
            return 0;
        }
    }

    string context = "Available commands: tickfile <file_name>";
    string contextIfSelected = "/print <start time> <end time> <symbol>/product <start time> <end time> <symbol> <field1> <field2>";
    string prompt = "$> ";
    bool isFileSelected = false;
    string cmd, file, currentFile;
    time_t s,e;
    string sym,f1,f2;
    while(1)
    {
        string line;
        cout << prompt << context << (isFileSelected ? contextIfSelected : "") << (isFileSelected ? "\tworking on:\033[1;32m\"" + currentFile + "\"\033[0m": "") << "\n" << prompt;
        getline(cin, line);

        stringstream inputStr(line);
        inputStr >> cmd;
        if(cmd == "print")
        {
            if(isFileSelected)
            {
                inputStr >> s >> e >> sym;
                parser->print(s, e, sym);
            }
        }
        else if(cmd == "product")
        {
            if(isFileSelected)
            {
                inputStr >> s >> e >> sym >> f1 >> f2;
                parser->product(s, e, sym, f1, f2);
            }
        }
        else if(cmd == "tickfile")
        {
            inputStr >> file;
            isFileSelected = parser->openTickFile(file);
            if(isFileSelected)
                currentFile = file;
        }
        else
            cout << prompt << " nope \n";
        line = string{};
    }
    return 0;
}
