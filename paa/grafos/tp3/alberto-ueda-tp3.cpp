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

#define JOKER '-'
#define ONE '1'
#define ZERO '0'

#define TRUE "true"
#define FALSE "false"
#define BOTH "both"

#define COMMAND_1 "11111"
#define COMMAND_0 "000"

#define SPECIAL_JOKER_1 "1111-00"
#define SPECIAL_JOKER_2 "00-1111"

#define npos string::npos

using namespace std;

bool find_command(string message)
{
    if (message.find(COMMAND_0) != npos || message.find(COMMAND_1) != npos)
    {
        return true;
    }

    return false;
}

bool find_special_joker(string message)
{
    if (message.find(SPECIAL_JOKER_1) != npos || message.find(SPECIAL_JOKER_2) != npos)
    {
        return true;
    }

    return false;
}

void generate_all_possible_inputs(string new_message, vector<string>* all_possibilities)
{
    int joker_index = new_message.find('-');

    if (joker_index  == npos) {
        cout << new_message << endl;
        (*all_possibilities).push_back(new_message);
    }
    else
    {
        string new_message_with_1 = new_message;
        string new_message_with_x = new_message;

        new_message[joker_index] = '0';
        generate_all_possible_inputs(new_message, all_possibilities);

        new_message_with_1[joker_index] = '1';
        generate_all_possible_inputs(new_message_with_1, all_possibilities);

        new_message_with_x[joker_index] = '*';
        generate_all_possible_inputs(new_message_with_x, all_possibilities);
    }

}

void generate_all_possibilities(string new_message, vector<string>* all_possibilities)
{
    int joker_index = new_message.find('-');

    if (joker_index  == npos) {
        // cout << new_message << endl;
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

    if (message.find('-') == npos) {
        return FALSE;
    }

    for (int i = 0; i < message.size(); ++i)
    {
        // cout << "    i = " << i << endl;
        if (message.find('-') == npos) {
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
                // cout << endl << "both to: " << all_possible_messages[k] << endl;
                return BOTH;
            }
        }

    }

    return FALSE;
}

char find_next_identified_bit(string message, int begin_index)
{
    for (int i = begin_index, k = 0; i < message.size() && k < 4; ++i, ++k)
    {
        if (message[i] != JOKER)
        {
            return message[i];
        }
    }

    return JOKER;
}

string greedy(const string message)
{
    // check if message has a control command, disconsidering errors
    if (find_command(message) || find_special_joker(message))
    {
        return TRUE;
    }

    if (message.find('-') == npos) {
        return FALSE;
    }

    if (message.find("---") != npos || message.find("0--") != npos
        || message.find("-0-") != npos || message.find("--0") != npos) {
        return BOTH;
    }

    string new_message = message;
    int counter_0 = 0;
    int counter_1 = 0;

    // If 1st char is a joker
    if (new_message[0] == JOKER)
    {
        char first_identified = find_next_identified_bit(new_message, 0);

        switch (first_identified)
        {
            case ONE:
                new_message[0] = ONE;
                ++counter_1;
                break;
            case ZERO:
            case JOKER:
                ++counter_0;
                new_message[0] = ZERO;
        }
    }
    else if (new_message[0] == ONE)
    {
        ++counter_1;
        counter_0 = 0;
    }
    else if (new_message[0] == ZERO)
    {
        ++counter_0;
        counter_1 = 0;
    }

    // 2nd to N chars
    for (int i = 1; i < new_message.size(); ++i)
    {

        if (new_message[i] == ONE)
        {
            ++counter_1;
            counter_0 = 0;
        }
        else if (new_message[i] == ZERO)
        {
            ++counter_0;
            counter_1 = 0;
        }
        else if (new_message[i] == JOKER)
        {
            // decision by previous bits only
            if (counter_1 > 3 || counter_0 > 1)
                return BOTH;

            // new_message[i] is a joker
            char next_identified = find_next_identified_bit(new_message, i+1);
            // cout << message << " (next) --> " << next_identified << endl;

            char previous_identified = new_message[i-1];

            switch (next_identified)
            {
                case ONE:
                    if (previous_identified == ONE)
                    {
                        new_message[i] = ONE;
                        ++counter_1;
                        counter_0 = 0;
                    }
                    break;

                case ZERO:
                    if (previous_identified == ZERO)
                    {
                        new_message[i] = ZERO;
                        ++counter_0;
                        counter_1 = 0;
                    }
                    break;

                case JOKER:
                    new_message[i] = ZERO;
                    ++counter_0;
                    counter_1 = 0;
            }
        }

        string submessage = new_message.substr(0, i+1);
        // cout << endl << "submessage: " << submessage << endl;

        if (find_command(submessage))
        {
            return BOTH;
        }
    }

    // cout << endl << "greedy result message: " << new_message << endl;

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
    string filename;

    if (argc == 1)
    {
        filename = "all_input_n_7.txt"; // experiment_paulo  input  all_7_input
    }
    else if (argc > 1)
    {
        filename = argv[1];
    }

    inputFile.open(filename.c_str());

    if (!inputFile.is_open())
    {
        cout << "File [" <<  filename << "] not found." << endl;
        return 1;
    }

    // Read Number of Instances
    string first_line;
    getline(inputFile, first_line);
    int number_of_instances = atoi(first_line.c_str());

    /*
    vector<string> answers_bf;
    vector<string> answers_ga;
    answers_bf.reserve(100);
    answers_ga.reserve(100);
    */

    // Read Instances
    for (int i = 0; i < number_of_instances; ++i)
    {
        string instance;
        getline(inputFile, instance);

        string answer_bf = brute_force(instance);
        string answer_ga = greedy(instance);

        if (answer_bf != answer_ga)
        {
            cout << i << "! [ " << instance << " ] : " << answer_bf << " X " << answer_ga << endl;
        }

        // cout << brute_force(instance) << endl;
        // cout << greedy(instance) << endl;
    }

    inputFile.close();

    /*
    // Generates test cases
    vector<string> all_possible_messages;
    generate_all_possible_inputs("---", &all_possible_messages);
    */
}
