/** ------------------------------------------------
Universidade Federal de Minas Gerais
Projeto e Analise de Algoritmos - 2014/2
3rd module - Paradigms

TP3 - An Spy History - Dynamic Programming
Student: Alberto Hideki Ueda
Student ID: 2014765817
------------------------------------------------- **/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <map>

#define JOKER '-'
#define ONE '1'
#define ZERO '0'

#define TRUE "true"
#define FALSE "false"
#define BOTH "both"

#define COMMAND_1 "11111"
#define COMMAND_0 "000"

#define MAX_MSG_SIZE 100

#define npos string::npos

using namespace std;

// Global variables
string message;
vector<vector<vector<int> > > dp_table;

void init_dp_table()
{
    dp_table.resize(MAX_MSG_SIZE + 1);

    for (int i = 0; i < dp_table.size(); i++)
    {
        dp_table[i].resize(6);

        for (int j = 0; j < 6; j++)
        {
            dp_table[i][j].resize(4);
        }
    }
}

void reset_dp_table()
{
    for (int i = 0; i < dp_table.size(); i++)
    {
        for (int j = 0; j < 6; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                dp_table[i][j][k] = 0;
            }
        }
    }
}

// 0 = unknow, 1 = false, 2 = both, 3 = true
int memoized_opt(int i, int counter_1, int counter_0)
{
    if (dp_table[i][counter_1][counter_0] != 0)
    {
        return dp_table[i][counter_1][counter_0];
    }

    int answer = 0;

    if (counter_1 == 5 || counter_0 == 3)
    {
        answer = 3;
    }
    else if (i == message.size())
    {
        answer = 1;
    }
    else if (message[i] == ONE)
    {
        answer = memoized_opt(i+1, counter_1+1, 0);
    }
    else if (message[i] == ZERO)
    {
        answer = memoized_opt(i+1, 0, counter_0+1);
    }
    else
    {
        int answer_with_0 = memoized_opt(i+1, 0, counter_0+1);
        int answer_with_1 = memoized_opt(i+1, counter_1+1, 0);

        if (answer_with_0 != answer_with_1)
        {
            answer = 2;
        }
        else
        {
            answer = answer_with_0;
        }
    }

    dp_table[i][counter_1][counter_0] = answer;

    return answer;
}

string dynamic_programming()
{
    int answer = memoized_opt(0, 0, 0);

    if (answer == 2) return BOTH;
    if (answer == 3) return TRUE;

    return FALSE;
}

void print_nodes(const char* filename, vector<int>* nodes)
{
    ofstream file(filename);
    if (file.is_open())
    {
        for (int i = 0; i < (*nodes).size(); i++)
        {
            file << (*nodes)[i] << endl;
        }
    }
    file.close();
}

int main(int argc, char** argv)
{
    ifstream input_file;
    string filename;

    if (argc == 1)
    {
        filename = "input.txt";
    }
    else if (argc > 1)
    {
        filename = argv[1];
    }

    // Open Input File
    input_file.open(filename.c_str());

    if (!input_file.is_open())
    {
        cout << "File [" <<  filename << "] not found." << endl;
        return 1;
    }

    // Read Number of Instances
    string first_line;
    getline(input_file, first_line);
    int number_of_instances = atoi(first_line.c_str());

    // Open Output File
    ofstream output_file("output.txt");

    if (!output_file.is_open())
    {
        cout << "Problem has occured on creating file [output.txt]." << endl;
        return 1;
    }

    init_dp_table();

    // Read Instances
    for (int i = 0; i < number_of_instances; ++i)
    {
        getline(input_file, message);
        output_file << dynamic_programming() << endl;

        reset_dp_table();
    }

    output_file.close();
    input_file.close();
}
