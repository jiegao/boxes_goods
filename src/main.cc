#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include "boxes_goods_checker.h"
using namespace std;

void string_split(const string& str, const string& sep, vector<int>& lst)
{
    if (str.empty()) return;
    string::size_type begp = 0;
    while (true)
    {
        string::size_type endp = str.find(sep, begp);
        string sub = str.substr(begp, endp - begp);
        if (!sub.empty()) {
            char *endptr = NULL;
            int value = strtol(sub.c_str(), &endptr, 10);
            if (strcmp(endptr, "") == 0)
                lst.push_back(value);
            else
                cout << "WARNING: invalid float value " << sub << ", skipped.\n";
        }
        if (endp == string::npos) break;
        begp = endp + sep.size();
    }
}

void print_usage()
{
    cout << "checkBoxesGoods usage:\n";
    cout << "- input: input file including boxes size and goods size, format:\n";
    cout << "         boxes_size: sz1, sz2, sz3, ...\n";
    cout << "         goods_size: sz1, sz2, sz3, sz4, sz5, ...\n";
    cout << "- dosimplecheck: firstly checking with some simple rules or not, default is disabled.\n";
}

int main(int argc, char* argv[])
{
    string infile("");
    int simple_check = 0;

    try
    {
        for (int i = 1; i < argc; ++i)
        {
            string o(argv[i]);
            if (o == "-input") {
                if (i < argc - 1) {
                    ++i;
                    infile = string(argv[i]);
                }
            }
            if (o == "-dosimplecheck") {
                if (i < argc - 1) {
                    ++i;
                    simple_check = atoi(argv[i]);
                }
            }
            if (o == "-h" || o == "-help") {
                print_usage();
                return 0;
            }
        }

        if (infile.empty()) {
            throw runtime_error("input file is not given!");
        } else {
            cout << "input file: " << infile << "\n";
        }

        ifstream input(infile.c_str(), ios::in);
        if (!input) {
            throw runtime_error("input file " + infile + " doesn't exist!");
        }

        char c;
        string line;
        string box_key("boxes_size: ");
        string good_key("goods_size: ");
        vector<int> boxes;
        vector<int> goods;
        boxes.reserve(64);
        goods.reserve(64);
        vector<vector<int> > boxes_list;
        vector<vector<int> > goods_list;
        boxes_list.reserve(256);
        goods_list.reserve(256);
        while (getline(input, line))
        {
            istringstream is(line);
            if (is >> c) {
                if (c == box_key[0] && boxes.empty()) {
                    size_t pos = line.find(box_key);
                    if (pos != string::npos) {
                        string size_str = line.substr(pos + box_key.size());
                        string_split(size_str, "," , boxes);
                        boxes_list.push_back(boxes);
                        boxes.clear();
                    }
                }
                if (c == good_key[0] && goods.empty()) {
                    size_t pos = line.find(good_key);
                    if (pos != string::npos) {
                        string size_str = line.substr(pos + good_key.size());
                        string_split(size_str, "," , goods);
                        goods_list.push_back(goods);
                        goods.clear();
                    }
                }
            }
        }

        if (boxes_list.size() != goods_list.size()) {
            throw runtime_error("boxes_size and goods_size are not matched in input file!");
        } else {
            Checker checker(simple_check);
            for (int i = 0; i < int(boxes_list.size()); ++i)
            {
                checker.init(boxes_list[i], goods_list[i]);
                checker.run();
            }
        }
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "***ERROR: %s\n", e.what());
        return 1;
    }
    catch (...)
    {
        fprintf(stderr, "***ERROR: unspecific error\n");
        return 1;
    }
    return 0;
}
