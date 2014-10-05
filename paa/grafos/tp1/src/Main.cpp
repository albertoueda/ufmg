#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#define RATIO 10

using namespace std;

typedef struct
{
    // friendship
    int f;

    // distance
    int d;

    double weight;

} Node;

typedef vector<vector<Node> > Graph;

void printMatrix(vector<vector<Node> > matrix)
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

vector<vector<Node> > initializeMatrix(int n, bool randomized)
{
    // 1-based
    int size = n + 1;
    vector<vector<Node> > matrix(size);

    for (int i = 1; i < size; i++) {
        matrix[i].resize(size);

        if (randomized)
            for (int j = 1; j < size; j++)
                matrix[i][j].f = rand() % n;
    }

    return matrix;
}

vector<vector<Node> > initializeMatrix(int n) {
    return initializeMatrix(n, false);
}

void visit(Graph g, int root, vector<int>* visited)
{
    for (int i = 1; i < g.size(); i++) {
        if (g[root][i].f != 0 && (*visited)[i] == 0) {
            (*visited)[i] = 1;

            cout << "Visited [" << root << "] -> [" << i << "]..." << endl;
            visit(g, i, visited);
        }
    }
}

vector<int> single_dfs(Graph g) {
    vector<int> visited(g.size());

    // to check if...
    visited[1] = 1;

    visit(g, 1, &visited);

    return visited;
}

bool connected_graph(Graph g)
{
    vector<int> visited_nodes = single_dfs(g);

    for (int i = 1; i < visited_nodes.size(); i++) {
        if (visited_nodes[i] == 0) {
            cout << "Node " << i << " not visited." << endl;
            return false;
        }
    }

    return true;
}

vector<vector<Node> > kruskal(vector<vector<Node> > graph)
{
    // each v -> new group(v)
    // sort edges decreasingly
    // while MST < N-1
    //      choose the maximum cost edge e
    //      if group(A) != group (B)
    //          add e to MST
    //          group(B) -> group(A)
    // return MST
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
    vector<vector<Node> > matrix;
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

            int i, j, f, d, w;
            stream >> i >> j >> f >> d;

            w = d - (f * RATIO);

            matrix[i][j].f = f;
            matrix[i][j].d = d;
            matrix[i][j].weight = w;
        }

        cout << "graph(" << n << "): " << connected_graph(matrix) << endl;

        printMatrix(matrix);
    }

    inputFile.close();


    // Output Ratio
    cout << returnRatio();
}

