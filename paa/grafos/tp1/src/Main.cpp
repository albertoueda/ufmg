#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>

# define EPSILON 0.0000001

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
    cout << endl;

    for (int i = 1; i < size; i++)
    {
        for (int j = 1; j < size; j++)
        {

            if (all_info)
            {
                cout << g[i][j].f << '/' << g[i][j].d << '/' << g[i][j].weight << ' ';
            }
            else
            {
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

    for (int i = 0; i < n; i++)
    {
        g[i].resize(n);

        if (randomized)
            for (int j = 0; j < n; j++)
                g[i][j].f = rand() % n;
    }

    return g;
}

Graph initialize_graph(int n)
{
    return initialize_graph(n, false);
}

void visit(Graph g, int root, vector<int>* visited)
{
    for (int i = 1; i < g.size(); i++)
    {
        if (g[root][i].f != 0 && (*visited)[i] == 0)
        {
            (*visited)[i] = 1;

            // cout << "Visited [" << root << "] -> [" << i << "]..." << endl;
            visit(g, i, visited);
        }
    }
}

vector<int> single_dfs(Graph g)
{
    vector<int> visited(g.size());

    // to check if...
    visited[1] = 1;

    visit(g, 1, &visited);

    return visited;
}

bool connected_graph(Graph g)
{
    vector<int> visited_nodes = single_dfs(g);

    for (int i = 1; i < visited_nodes.size(); i++)
    {
        if (visited_nodes[i] == 0)
        {
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

    while (i < size)
    {
        if (iLeft == left.size() || iRight == right.size())
        {
            break;
        }

        if (left[iLeft].weight > right[iRight].weight)
        {
            sorted[i++] = left[iLeft++];
        }
        else
        {
            sorted[i++] = right[iRight++];
        }
    }

    // Just at maximum one of the conditions below will be true
    while (iRight < right.size())
    {
        sorted[i++] = right[iRight++];
    }

    while (iLeft < left.size())
    {
        sorted[i++] = left[iLeft++];
    }

    return sorted;
}

// fix
vector<Edge> mergesort(vector<Edge> edges, int p, int q)
{
    if (p < q)
    {
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
    for (int i = 1; i < colors.size(); i++)
    {
        colors[i] = i;
    }

    // get all edges
    vector<Edge> all_edges;
    for (int i = 1; i < V; i++)
    {
        for (int j = i + 1; j < V; j++)  // j = i ?
        {
            if (graph[i][j].d != 0)   // dangerous != 0
            {
                all_edges.push_back(graph[i][j]);
                // all_edges.push_back(graph[j][i]);
                // cout << "Inserting [" << i << "] -> [" << j << "] to collection" << endl;
            }
        }
    }

    // sort edges decreasingly
    vector<Edge> sorted_edges = mergesort(all_edges, 0, all_edges.size() - 1);
    /*
    cout << "sorted edges: ";
    for (int i = 0; i < sorted_edges.size(); i+=2) // sorted duplicate
    {
        cout << sorted_edges[i].weight << ' ';
    }
    cout << endl;
    */

    int k = 0, mst_size = 0;
    Graph mst = initialize_graph(V);

    while (mst_size < V-2 && k < colors.size())
    {
        Edge candidate = sorted_edges[k++];

        if (colors[candidate.source] != colors[candidate.target])
        {
            mst[candidate.source][candidate.target].weight = 1;
            mst_size++;

            int old_color = colors[candidate.target];
            int new_color = colors[candidate.source];

            // make union based on source color
            for (int i = 1; i < colors.size(); i++)
            {
                if (colors[i] == old_color)
                {
                    colors[i] = new_color;
                }
            }

            /*
            cout << "colors: ";
            for (int i = 1; i < colors.size(); i++) {
                cout << colors[i] << ' ';
            }
            cout << endl;

            cout << "mst:" << endl;
            print_graph(mst);
            cout << endl;
            */
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

//    // Initialization
//    Graph g;
//    int n;
//    double quality_candidate = 0.0;

    // Input Reader
    string line;
    ifstream inputFile;
    inputFile.open("entrada1.txt");

    if (!inputFile.is_open())
    {
        return 0;
    }

    // Input File Reader
    while (getline(inputFile, line))
    {

        if (isdigit(line[0]) == false)
        {
            break;
        }

        // number of nodes
        int n = atoi(line.c_str());

        // 1-based: n + 1
        Graph g = initialize_graph(n + 1);

        double lower_bound = 0.0;
        double upper_bound = 0.0;
        double quality_candidate = 0.0;
        double best_quality = 0.0;

        // graph info reader. Last line read is a blank line
        while (getline(inputFile, line) && isdigit(line[0]))
        {
            stringstream stream(line);

            int i, j, f, d, w;
            stream >> i >> j >> f >> d;

            // increases first upper bound to binary search
            upper_bound += f;

            g[i][j].f = f;
            g[i][j].d = d;
            g[i][j].source = i;
            g[i][j].target = j;

            // the inversed edge also is added
            g[j][i].f = f;
            g[j][i].d = d;
            g[j][i].source = j;
            g[j][i].target = i;
        }

        if (!connected_graph(g))
        {
            quality_candidate = -1.000;
            cout << "graph(" << n << ") -> best quality: -1.000" << endl;
            cout << "**************************************************************" << endl;
            continue;
        }

        // Determine the best quality using a binary search
        while (abs(upper_bound - lower_bound) > EPSILON)
        {
            // calculates the next candidate for quality
            quality_candidate = (upper_bound + lower_bound) / 2;
            // cout << "quality candidate: " << quality_candidate << endl;

            double sum_weights = 0.0;

            // horror
            // define a new weight to edges based on new quality
            for (int i = 1; i < g.size(); i++)
            {
                for (int j = i + 1; j < g.size(); j++)  // j = i ?
                {
                    int f = g[i][j].f;
                    int d = g[i][j].d;

                    if (d != 0)   // dangerous != 0
                    {
                        int new_weight = f - (d * quality_candidate);

                        g[i][j].weight = new_weight;
                        g[j][i].weight = new_weight;
                        // cout << "Inserting [" << i << "] -> [" << j << "] to collection" << endl;

                        // sum_weights += new_weight;
                    }
                }
            }

            // print_graph(g);

            Graph candidate = kruskal(g);

            // add edges that will increase the ratio
            // if  w > 0  =>  f - r*d > 0  =>  f > r*d   =>  f/d > r
            for (int i = 1; i < candidate.size(); i++)
            {
                for (int j = i + 1; j < candidate.size(); j++)
                {
                    if (candidate[i][j].weight == 1)
                    {
                        sum_weights += g[i][j].weight;
                    }
                    else if (candidate[i][j].weight == 0 && g[i][j].weight > 0)
                    {
                        candidate[i][j].weight = 1;
                        sum_weights += g[i][j].weight;
                    }
                }
            }

            // print_graph(candidate);

            // stop if quality is good enough
            /*
            cout << "sum of weights: " << sum_weights << endl;
            cout << "-------------------------------" << endl;
            */

            if (abs(sum_weights) < EPSILON)
            {
                cout << "sum_weights < " << EPSILON << "!\n\n";
                break;
            }

            // if is not, calculates the new lower and topper bounds
            if (sum_weights > 0)
            {
                lower_bound = quality_candidate;
            }
            else
            {
                upper_bound = quality_candidate;
            }
        }

        best_quality = quality_candidate;

        if (abs(upper_bound - lower_bound) < EPSILON) {
            cout << "upper_bound - lower_bound < " << EPSILON << "!\n\n";
        }

        // Output Ratio
        cout << "graph(" << n << ") -> best quality: " << best_quality << endl;
        cout << "**************************************************************" << endl;
    }

    inputFile.close();

}

