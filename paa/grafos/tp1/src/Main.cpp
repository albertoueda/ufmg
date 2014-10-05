#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#define RATIO 10

using namespace std;

typedef struct
{
    // friendship and distance
    int f;
    int d;

    // adopted weight based on 'f' and 'd'
    double weight;

    // useful for Kruskal
    int source;
    int target;

} Edge;

typedef vector<vector<Edge> > Graph;

void print_graph(Graph g, bool all_info)
{
    int size = g.size();

    for (int i = 1; i < size; i++) {
        for (int j = 1; j < size; j++) {

            if (all_info) {
                cout << g[i][j].f << '/' << g[i][j].d << '/' << g[i][j].weight << ' ';
            } else {
                cout << g[i][j].weight << ' ';
            }

        }
        cout << endl;
    }
    cout << endl;
}

void print_graph(Graph g)
{
    print_graph(g, false);
}

Graph initialize_graph(int n, bool randomized)
{
    Graph g(n);

    for (int i = 0; i < n; i++) {
        g[i].resize(n);

        if (randomized)
            for (int j = 0; j < n; j++)
                g[i][j].f = rand() % n;
    }

    return g;
}

Graph initialize_graph(int n) {
    return initialize_graph(n, false);
}

void visit(Graph g, int root, vector<int>* visited)
{
    for (int i = 1; i < g.size(); i++) {
        if (g[root][i].f != 0 && (*visited)[i] == 0) {
            (*visited)[i] = 1;

            // cout << "Visited [" << root << "] -> [" << i << "]..." << endl;
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
            cout << "Node " << i << " not visited. The graph is not connected." << endl;
            return false;
        }
    }

    return true;
}

vector<Edge> merge(vector<Edge> left, vector<Edge> right)
{
    int i = 0, iLeft = 0, iRight = 0;
    int size = left.size() + right.size();
    vector<Edge> sorted(size);

    while (i < size) {
        if (iLeft == left.size() || iRight == right.size()) {
            break;
        }

        if (left[iLeft].weight > right[iRight].weight) {
            sorted[i++] = left[iLeft++];
        } else {
            sorted[i++] = right[iRight++];
        }
    }

    // Just at maximum one of the conditions below will be true
    while (iRight < right.size()) {
        sorted[i++] = right[iRight++];
    }

    while (iLeft < left.size()) {
        sorted[i++] = left[iLeft++];
    }

    return sorted;
}

// fix
vector<Edge> mergesort(vector<Edge> edges, int p, int q)
{
    if (p < q) {
        int r = (p + q) / 2;
        vector<Edge> left = mergesort(edges, p, r);
        vector<Edge> right = mergesort(edges, r + 1, q);

        return merge(left, right);
    }

    vector<Edge> single_vector(1);
    single_vector[0] = edges[p];

    return single_vector;
}

Graph kruskal(Graph graph)
{
    int V = graph.size();

    // each v -> new group(v)
    vector<int> colors(V);
    for (int i = 1; i < colors.size(); i++) {
        colors[i] = i;
    }

    // get all edges
    vector<Edge> all_edges;
    for (int i = 1; i < V; i++) {
        for (int j = 1; j < V; j++) {
            if (graph[i][j].d != 0) { // dangerous != 0
                all_edges.push_back(graph[i][j]);
                // cout << "Inserting [" << i << "] -> [" << j << "] to collection" << endl;
            }
        }
    }

    // sort edges decreasingly
    vector<Edge> sorted_edges = mergesort(all_edges, 0, all_edges.size() - 1);
    cout << "sorted edges: ";
    for (int i = 0; i < sorted_edges.size(); i++) {
        cout << sorted_edges[i].weight << ' ';
    }
    cout << "\n\n";

    int k = 0, mst_size = 0;
    Graph mst = initialize_graph(V);

    while (mst_size < V-2 && k < colors.size()) {
        Edge candidate = sorted_edges[k++];

        if (colors[candidate.source] != colors[candidate.target]) {
            mst[candidate.source][candidate.target].weight = 1;
            mst_size++;

            int old_color = colors[candidate.target];
            int new_color = colors[candidate.source];

            // make union based on source color
            for (int i = 1; i < colors.size(); i++) {
                if (colors[i] == old_color) {
                    colors[i] = new_color;
                }
            }

            cout << "colors: ";
            for (int i = 1; i < colors.size(); i++) {
                cout << colors[i] << ' ';
            }
            cout << endl;

            cout << "mst:" << endl;
            print_graph(mst);
            cout << endl;

        }
    }

    return mst;
}

double return_ratio()
{
    return 2.42;
}

int main()
{
    // initialize random seed:
    srand (time(NULL));

    // Initialization
    Graph g;
    int n;
    double best_quality;

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

        // 1-based: n + 1
        g = initialize_graph(n + 1);

        while (getline(inputFile, line) && isdigit(line[0])) {
            stringstream stream(line);

            int i, j, f, d, w;
            stream >> i >> j >> f >> d;

            w = f - (d * RATIO);

            g[i][j].f = f;
            g[i][j].d = d;
            g[i][j].weight = w;

            g[i][j].source = i;
            g[i][j].target = j;
        }

        print_graph(g);

        if (!connected_graph(g)) {
            best_quality = -1.000;
        } else {
            kruskal(g);

            best_quality = return_ratio();
        }

        // Output Ratio
        cout << "graph(" << n << ") -> best quality: " << best_quality << endl;
        cout << "**************************************************************" << endl;
    }

    inputFile.close();

}

