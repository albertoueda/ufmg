#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>

# define EPSILON 0.0001

using namespace std;

typedef struct cell
{
    // node id
    int v;

    // friendship and distance
    int f;
    int d;

    // adopted weight based on 'f' and 'd'
    double weight;

    // useful for Kruskal
    int parent;

    // Next element in the linked list
    struct cell* next;

} *Cell;

typedef vector<Cell> Graph;

Graph initialize_graph(int n, bool randomized)
{
    Graph g(n);

    for (int i = 0; i < n; i++)
    {
        g[i] = NULL;

    Cell new_cell = (Cell) malloc(sizeof(Cell));
    new_cell->v = 1;
    // new_cell->weight = 1;
    new_cell->next = NULL;
    g[i] = new_cell;
    //        if (randomized)
    //            for (int j = 0; j < n; j++)
    //                g[i][j].f = rand() % n;
    }

    return g;
}

Graph initialize_graph(int n)
{
    return initialize_graph(n, false);
}

void insert(Graph g, int parent, Cell new_cell)
{
    // Push front the new cell to g[i]
    new_cell->next = g[parent];
    g[parent] = new_cell;
}

// Always insert
void insert(Graph g, int i, int j, int f, int d)
{
    Cell new_cell = (Cell) malloc(sizeof(Cell));
    new_cell->parent = i;
    new_cell->v = j;
    new_cell->f = f;
    new_cell->d = d;

    insert(g, i, new_cell);
}

bool exists_edge(Cell linked_list, int target)
{
    for (Cell cell = linked_list; cell != NULL; cell = cell->next)
    {
        if (cell->v == target) {
            return true;
        }
    }

    return false;
}

bool exists_edge(Graph g, int source, int target)
{
    return exists_edge(g[source], target);
}

void print_linked_list(Cell linked_list, bool show_all_info)
{
    Cell cell;

    for (cell = linked_list; cell != NULL; cell = cell->next)
    {
        if (show_all_info)
        {
            cout << cell->v << ":" << cell->f << '/' << cell->d << ':' << cell->weight << ' ';
        }
        else
        {
            cout << cell->v << ":" << cell->weight << ' ';
        }
    }
    cout << endl;
}

void print_graph(Graph g, bool show_all_info)
{
    cout << endl;

    for (int i = 1; i < g.size(); i++)
    {
        cout << i << ": ";
        print_linked_list(g[i], show_all_info);
    }

    cout << endl;
}

void print_graph(Graph g)
{
    print_graph(g, false);
}

void visit(Graph g, int source, vector<int>* visited)
{
    for (int j = 1; j < g.size(); j++)
    {
        if (exists_edge(g, source, j) && (*visited)[j] == 0)
        {
            (*visited)[j] = 1;

            // cout << "Visited [" << root << "] -> [" << i << "]..." << endl;
            visit(g, j, visited);
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

vector<Cell> merge(vector<Cell> left, vector<Cell> right)
{
    int i = 0, iLeft = 0, iRight = 0;
    int size = left.size() + right.size();
    vector<Cell> sorted(size);

    while (i < size)
    {
        if (iLeft == left.size() || iRight == right.size())
        {
            break;
        }

        if (left[iLeft]->weight > right[iRight]->weight)
        {
            sorted[i++] = left[iLeft++];
        }
        else
        {
            sorted[i++] = right[iRight++];
        }
    }

    // At maximum one of the conditions below will be true
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

vector<Cell> mergesort(vector<Cell> cells, int p, int q)
{
    if (p < q)
    {
        int r = (p + q) / 2;
        vector<Cell> left = mergesort(cells, p, r);
        vector<Cell> right = mergesort(cells, r + 1, q);

        return merge(left, right);
    }

    // p == q. Just creates a vector of one element
    vector<Cell> single_vector(1);
    single_vector[0] = cells[p];

    return single_vector;
}

Graph kruskal(Graph g)
{
    int V = g.size();

    // each v -> new group(v)
    vector<int> colors(V);
    for (int i = 1; i < colors.size(); i++)
    {
        colors[i] = i;
    }

    // get all edges
    vector<Cell> all_edges;
    for (int i = 1; i < V; i++)
    {
        for (Cell cell = g[i]; cell != NULL; cell = cell->next)
        {
            all_edges.push_back(cell);
            // all_edges.push_back(graph[j][i]);
            // cout << "Inserting [" << i << "] -> [" << j << "] to collection" << endl;
        }
    }

    // sort edges decreasingly
    vector<Cell> sorted_edges = mergesort(all_edges, 0, all_edges.size() - 1);

    /*
    cout << "sorted edges: ";
    for (int i = 0; i < sorted_edges.size(); i+=2) // sorted duplicate
    {
        cout << sorted_edges[i]->weight << ' ';
    }
    cout << endl;
    */

    int k = 0, mst_size = 0;
    Graph mst = initialize_graph(V);

    while (mst_size < V-2 && k < colors.size())
    {
        Cell candidate = sorted_edges[k++];

        if (colors[candidate->parent] != colors[candidate->v])
        {
            insert(mst, candidate->parent, candidate);

            // For each node i, mst[i] must contain at most one element
            mst[candidate->parent]->next = NULL; // danger
            mst_size++;

            int old_color = colors[candidate->v];
            int new_color = colors[candidate->parent];

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

// define a new weight to edges based on new quality
void recalculate_weights(Graph g, double quality_candidate)
{
    for (int i = 1; i < g.size(); i++)
    {
        for (Cell cell = g[i]; cell != NULL; cell = cell->next)
        {
            int f = cell->f;
            int d = cell->d;

            double new_weight = f - (d * quality_candidate);

            cell->weight = new_weight;
            // cout << "Inserting [" << i << "] -> [" << j << "] to collection" << endl;
            // sum_weights += new_weight;
        }
    }
}


int main()
{
    // initialize random seed:
    srand (time(NULL));

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

            insert(g, i, j, f, d);
            insert(g, j, i, f, d);
        }

        print_graph(g);
/*
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

            recalculate_weights(g, quality_candidate);

            // print_graph(g);

            Graph candidate_graph = kruskal(g);

            // add edges of MST and also edges that will increase the ratio
            // if  w > 0  =>  f - r*d > 0  =>  f > r*d   =>  f/d > r
            for (int i = 1; i < candidate_graph.size(); i++)
            {
                for (Cell cell = g[i]; cell != NULL; cell = cell->next)
                {
                    if (exists_edge(candidate_graph, i, cell->v))
                    {
                        sum_weights += cell->weight;
                    }
                    else if (cell->weight > 0)
                    {
                        sum_weights += cell->weight;

                        // Just for visualization of solution graph
                        insert(candidate_graph, i, cell);
                    }
                }
            }


            // print_graph(candidate);

            // stop if equation equals zero
            // cout << "sum of weights: " << sum_weights << endl;
            // cout << "-------------------------------" << endl;


            if (abs(sum_weights) == 0)
            {
                cout << "sum_weights = zero!\n\n";
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
    */
    }

    inputFile.close();

}

