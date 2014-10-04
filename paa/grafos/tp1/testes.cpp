#include <cstdlib>
#include <iostream>

using namespace std;

int main_ANTIGO()
{
    string a = "Hello World! ";

    for ( int i = 0; i < 10; i++ )
    {
        cout << a << i << endl;
    }

    // initialize matrix
    int** matrix = (int**) malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        matrix[i] = (int*) malloc (n * sizeof(int));
    }

    printAdjacencyList(matrixC, n);
    printAdjacencyList(matrixC, n);

    // vector
    vector<int> a;
    a.reserve(10);

    string c;

    while (c != "x") {
        cin >> c;
        a.push_back(atoi(c.c_str()));

        for (int i = 0; i < a.size(); i++)
            cout << a[i] << ' ';
        cout << endl;
    }

    cout << a.at(0);
    return 0;
}

int nada()
{
    return 0;
}

void imprime(string a)
{
    cout << a << endl;
}

typedef struct Node {
    int data;
    struct Node *nextptr;
} node, *node_ptr;

or

typedef struct Node {
    int data;
    struct Node *nextptr;
} node;

typedef node* node_ptr;

or

typedef struct Node* node_ptr;

