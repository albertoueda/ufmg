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

#define MAX_MSG_SIZE 100

#define npos string::npos

using namespace std;

// Global variables
string message;
vector<vector<vector<int> > > dp_table;

bool find_command_as_bool(string message)
{
    if (message.find(COMMAND_0) != npos || message.find(COMMAND_1) != npos)
    {
        return true;
    }

    return false;
}

string find_command(string message)
{
    if (find_command_as_bool(message))
    {
        return TRUE;
    }

    return FALSE;
}

void generate_all_possible_inputs(string new_message, vector<string>* all_possibilities)
{
    int joker_index = new_message.find('-');

    if (joker_index  == npos)
    {
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

string check_all_possibilities(string new_message)
{
    int joker_index = new_message.find('-');

    if (joker_index == npos)
    {
        return find_command(new_message);
    }
    else
    {
        string new_message_with_1 = new_message;

        new_message[joker_index] = '0';
        string answer_with_0 = check_all_possibilities(new_message);


        // 4th Prunning
        if (answer_with_0 == BOTH)
        {
            return BOTH;
        }

        new_message_with_1[joker_index] = '1';
        string answer_with_1 = check_all_possibilities(new_message_with_1);


        // 5th Prunning
        if (answer_with_1 == BOTH)
        {
            return BOTH;
        }

/*
        cout << "comparing: \n" << new_message << " " << answer_with_0 << endl
            << new_message_with_1 << " "  << answer_with_1 << endl << endl;

        if (answer_with_0 == BOTH) cout << new_message << " " << answer_with_0 << endl;
        if (answer_with_1 == BOTH) cout << new_message_with_1 << " " << answer_with_1 << endl;
*/

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

string brute_force(string message)
{
    // 1st Prunning
    // check if message has a control command, disconsidering errors
    if (find_command_as_bool(message))
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

    return check_all_possibilities(message);
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

bool find_possible_command(string message, int begin_index)
{
    bool has_1 = false;
    bool has_0 = false;
    int k = 0;

    for (int i = begin_index; i < message.size() && k < 4; ++i, ++k)
    {
        if (message[i] == ONE)
        {
            has_1 = true;
        }
        else if (message[i] == ZERO)
        {
            has_0 = true;
        }
    }

    if (k != 4)
    {
        return false;
    }

    return (!has_1 || !has_0);
}

void choose_bit(string* message, int i, int* counter_1, int* counter_0)
{
    char next_identified = find_next_identified_bit(*message, i+1);
    char previous_identified;
    bool no_previous = false;

    int chosen_bit = 0;

    if ((*counter_1) > 3)
    {
        chosen_bit = 1;
    }
    else if ((*counter_0) > 1)
    {
        chosen_bit = 0;
    }
    else
    {
        if (i == 0)
        {
            no_previous = true;
        }
        else
        {
            previous_identified = (*message)[i-1];
        }

        switch (next_identified)
        {
            case ONE:
                if (no_previous || previous_identified == ONE)
                {
                    chosen_bit = 1;
                }
                break;

            case ZERO:
                if (no_previous || previous_identified == ZERO)
                {
                    chosen_bit = 0;
                }
                break;

            case JOKER:
                if (no_previous || previous_identified == ZERO)
                {
                    chosen_bit = 0;
                }
                else
                {
                    chosen_bit = 1;
                }
        }
    }

    if (chosen_bit == 1)
    {
        (*message)[i] = ONE;
        ++(*counter_1);
        (*counter_0) = 0;
    }
    else
    {
        (*message)[i] = ZERO;
        ++(*counter_0);
        (*counter_1 ) = 0;
    }

}

string find_command(int i, int counter_1, int counter_0, int last_joker_index)
{
    string answer = FALSE;
    // cout << i << " " << counter_1 << " " << counter_0 << " " << last_joker_index << endl;

    // Command with ONES
    if (counter_1 > 4)
    {
        if (last_joker_index < i-4)
        {
            return TRUE;
        }
        else
        {
            answer = BOTH;
        }
    }

    // Command with ZEROS
    if (counter_0 > 2)
    {
        if (last_joker_index < i-2)
        {
            return TRUE;
        }
        else
        {
            answer = BOTH;
        }
    }

    return answer;
}

// Accuracy = 99%
string greedy(const string message)
{
    // cout << endl

    int counter_0 = 0;
    int counter_1 = 0;
    int last_joker_index = -1;

    string final_answer = FALSE;
    string local_answer = FALSE;
    string new_message = message;

    for (int i = 0; i < new_message.size(); ++i)
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
            last_joker_index = i;
            choose_bit(&new_message, i, &counter_1, &counter_0);
        }

        local_answer = find_command(i, counter_1, counter_0, last_joker_index);

        if (local_answer == TRUE)
        {
            return TRUE;
        }
        else if (local_answer == BOTH)
        {
            final_answer = BOTH;
        }
    }

    // cout << "greedy final message: " << new_message << " " << answer << endl;

    return final_answer;
}

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
    ifstream inputFile;
    string filename;

    if (argc == 1)
    {
        filename = "input.txt"; // experiment_paulo_input  input  all_input_n_7  strings-tp-description
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

    init_dp_table();

    /*
    vector<string> answers_bf;
    vector<string> answers_ga;
    answers_bf.reserve(100);
    answers_ga.reserve(100);
    */
    int diff = 0;

    // Read Instances
    for (int i = 0; i < number_of_instances; ++i)
    {
        getline(inputFile, message);
        string bf_message = message;

        // string answer_bf = brute_force(bf_message);
        string answer_dp = dynamic_programming();
        // string answer_ga = greedy(instance);

        /*
        if (answer_bf != answer_dp)
        {
            cout << i << "! [ " << message << " ] : " << answer_bf << " X " << answer_dp << endl;
            ++diff;
        }
        */

        // cout << "DP [ " << message << " ] : " << dynamic_programming() << endl;
        // cout << "brute force [ " << instance << " ] : " << brute_force(instance) << endl;
        cout << dynamic_programming() << endl;

        reset_dp_table();
    }

    inputFile.close();

    /*
    cout << "Accuracy: " << (100 - (diff*100/number_of_instances)) << "%" << endl;

    // Generates test cases
    vector<string> all_possible_messages;
    generate_all_possible_inputs("------------", &all_possible_messages);
    */
}
