/** ------------------------------------------------
Universidade Federal de Minas Gerais
Projeto e Analise de Algoritmos - 2014/2
3rd module - Paradigms

TP3 - An Spy History
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

#define TRUE "true"
#define FALSE "false"
#define BOTH "both"
#define COMMAND_1 "11111"
#define COMMAND_0 "000"

using namespace std;


bool find_command(string message)
{
    if (message.find(COMMAND_0) != string::npos || message.find(COMMAND_1) != string::npos)
    {
        return true;
    }

    return false;
}


string brute_force(string message)
{
    // check if message has a control command, disconsidering errors
    if (find_command(message))
    {
        return TRUE;
    }

    for (int i = 0; i < message.size(); ++i)
    {


        // generate all possible solutions for interval of 7

        // check each solution

        // go to next interval (step size: 1)

    }

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
    ifstream inputFile;

    if (argc == 1)
    {
        inputFile.open("input.txt");
    }
    else if (argc > 1)
    {
        inputFile.open(argv[1]);
    }

    if (!inputFile.is_open())
    {
        cout << "File [" <<  argv[1] << "] not found, neither default input file [input.txt]." << endl;
        return 1;
    }

    // Read Number of Instances
    string first_line;
    getline(inputFile, first_line);
    int number_of_instances = atoi(first_line.c_str());

    // Read Instances
    for (int i = 0; i < number_of_instances; ++i)
    {
        string instance;
        getline(inputFile, instance);

        cout << brute_force(instance) << endl;
    }

    inputFile.close();
}
