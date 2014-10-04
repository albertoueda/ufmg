#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

typedef struct
{
    // friendship
    int f;

    // distance
    int d;

    double weight;
} node;

void printMatrix(vector<vector<node> > matrix)
{
    int size = matrix.size();

    for (int i = 1; i < size; i++) {
        for (int j = 1; j < size; j++) {
            cout << matrix[i][j].f << '/' << matrix[i][j].d << '/' << matrix[i][j].weight << ' ';
        }
        cout << endl;
    }
    cout << endl;
}

vector<vector<node> > initializeMatrix(int n, bool randomized)
{
    // 1-based
    int size = n + 1;
    vector<vector<node> > matrix(size);

    for (int i = 1; i < size; i++) {
        matrix[i].resize(size);

        if (randomized)
            for (int j = 1; j < size; j++)
                matrix[i][j].f = rand() % n;
    }

    return matrix;
}


vector<vector<node> > initializeMatrix(int n) {
    return initializeMatrix(n, false);
}

double returnRatio()
{
    return 2.42;
}

int main()
{
    // initialize random seed:
    srand (time(NULL));

    // Initialization
    vector<vector<node> > matrix;
    int n;

    // Input Reader
    string line;
    ifstream inputFile;
    inputFile.open("entrada1.txt");

    if (!inputFile.is_open()) {
        return 0;
    }

    while (getline(inputFile, line)) {

        if (isdigit(line[0]) == false) {
            break;
        }

        // number of nodes
        n = atoi(line.c_str());

        matrix = initializeMatrix(n);

        while (getline(inputFile, line) && isdigit(line[0])) {
            stringstream stream(line);

            int i, j, f, d, w = 1.2;
            stream >> i >> j >> f >> d;

            matrix[i][j].f = f;
            matrix[i][j].d = d;
            matrix[i][j].weight = w;

        }

        printMatrix(matrix);
    }

    inputFile.close();


    // Output Ratio
    cout << returnRatio();
}

