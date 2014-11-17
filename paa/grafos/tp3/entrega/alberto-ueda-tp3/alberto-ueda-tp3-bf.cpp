/** ------------------------------------------------
Universidade Federal de Minas Gerais
Projeto e Analise de Algoritmos - 2014/2
3rd module - Paradigms

TP3 - An Spy History - Brute Force Algorithm
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
string bf_message;

bool find_command_as_bool(string* message)
{
    if (message->find(COMMAND_0) != npos || message->find(COMMAND_1) != npos)
    {
        return true;
    }

    return false;
}

string find_command(string* message)
{
    if (find_command_as_bool(message))
    {
        return TRUE;
    }

    return FALSE;
}

string check_all_possibilities(string* new_message)
{
    int joker_index = new_message->find('-');

    if (joker_index == npos)
    {
        return find_command(new_message);
    }
    else
    {
        string new_message_with_1 = *new_message;

        (*new_message)[joker_index] = '0';
        string answer_with_0 = check_all_possibilities(new_message);

        // 4th Prunning
        if (answer_with_0 == BOTH)
        {
            return BOTH;
        }

        new_message_with_1[joker_index] = '1';
        string answer_with_1 = check_all_possibilities(&new_message_with_1);

        // 5th Prunning
        if (answer_with_1 == BOTH)
        {
            return BOTH;
        }

        if (answer_with_0 == answer_with_1)
        {
            return answer_with_0;
        }
        else
        {
            return BOTH;
        }
    }
}

string brute_force()
{
    // 1st Prunning
    // check if message has a control command, disconsidering errors
    if (find_command_as_bool(&message))
    {
        return TRUE;
    }

    // 2nd Prunning
    if (message.find('-') == npos)
    {
        return FALSE;
    }

    // 3rd Prunning
    if (message.size() > 2 && message.find(ONE) == npos && message.find(ZERO) == npos)
    {
        return BOTH;
    }

    bf_message = message;

    return check_all_possibilities(&bf_message);
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

    // Read Instances
    for (int i = 0; i < number_of_instances; ++i)
    {
        getline(input_file, message);
        output_file << brute_force() << endl;
    }

    output_file.close();
    input_file.close();
}
