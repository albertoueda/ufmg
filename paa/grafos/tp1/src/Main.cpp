#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

#define N 10

using namespace std;

typedef struct
{
    // friendship
    int f;

    // distance
    int d;

    int weigth;
} node;

void printMatrix(vector<vector<node> > matrix, int n)   // &matrix
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << matrix[i][j].f << ' ';
        }
        cout << endl;
    }
    cout << endl;
}

vector<vector<node> > initializeMatrix(bool randomized)
{
    vector<vector<node> > matrix(N);

    for (int i = 0; i < N; i++) {
        matrix[i].resize(N);

        for (int j = 0; j < N; j++)
            if (randomized) {
                matrix[i][j].f = rand() % N;
            }

    }

    return matrix;
}


vector<vector<node> > initializeMatrix() {
    return initializeMatrix(false);
}

double returnRatio()
{
    return 2.42;
}

int main()
{
    // initialize random seed:
    srand (time(NULL));

    // Input Reader
    string line;
    ifstream inputFile;
    inputFile.open("entrada1.txt");

    if (inputFile.is_open()) {
        while (getline(inputFile, line)) {
            cout << line << endl;
        }
    }
    inputFile.close();


    vector<vector<node> > matrix = initializeMatrix();


    printMatrix(matrix, N);

    // Output Ratio
    cout << returnRatio();
}

