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
#define SPECIAL_JOKER_1 "1111-00"
#define SPECIAL_JOKER_2 "00-1111"

using namespace std;


bool find_command(string message)
{
    if (message.find(COMMAND_0) != string::npos || message.find(COMMAND_1) != string::npos)
    {
        return true;
    }

    return false;
}

bool find_special_joker(string message)
{
    if (message.find(SPECIAL_JOKER_1) != string::npos || message.find(SPECIAL_JOKER_2) != string::npos)
    {
        return true;
    }

    return false;
}

void generate_all_possibilities(string new_message, vector<string>* all_possibilities)
{
    int joker_index = new_message.find('-');

    if (joker_index  == string::npos) {
        // cout << "New possibility: " << new_message << endl;
        (*all_possibilities).push_back(new_message);
    }
    else
    {
        string new_message_with_1 = new_message;

        new_message[joker_index] = '0';
        generate_all_possibilities(new_message, all_possibilities);

        new_message_with_1[joker_index] = '1';
        generate_all_possibilities(new_message_with_1, all_possibilities);
    }

}

string brute_force(string message)
{
    // check if message has a control command, disconsidering errors
    if (find_command(message) || find_special_joker(message))
    {
        return TRUE;
    }

    if (message.find('-') == string::npos) {
        return FALSE;
    }

    for (int i = 0; i < message.size(); ++i)
    {
        // cout << "    i = " << i << endl;
        if (message.find('-') == string::npos) {
            continue;
        }

        string submessage = message.substr(i, 5);

        vector<string> all_possible_messages;
        all_possible_messages.reserve(pow(2, 5));
        generate_all_possibilities(submessage, &all_possible_messages);

        for (int k = 0; k < all_possible_messages.size(); ++k)
        {
            if (find_command(all_possible_messages[k]))
            {
                return BOTH;
            }
        }

    }

    return FALSE;
}

string greedy(string message)
{
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

        cout << greedy(instance) << endl;
        // cout << brute_force(instance) << endl;
    }

    inputFile.close();
}
