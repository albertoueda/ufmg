/** ------------------------------------------------
Universidade Federal de Minas Gerais
Projeto e Analise de Algoritmos - 2014/2
2nd module - Graphs

TP2 - The Bow-Tie Structure of Web
Student: Alberto Hideki Ueda
Student ID: 2014765817
------------------------------------------------- **/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>

#define INITIAL_CAPACITY 500000

using namespace std;

// An edge
struct cell
{
    // node id
    int v;

    int parent;

    // Next element in the linked list
    struct cell* next;
};

struct SCC
{
    // node id
    vector<int> nodes;

    int type;
};

typedef struct cell* Cell;

typedef vector<Cell> Graph;

typedef struct SCC* sccPointer;

Graph initialize_graph(int n, bool randomized)
{
    Graph g(n);
    for (int i = 1; i < n; i++)
    {
        g[i] = NULL;
    }

    return g;
}

Graph initialize_graph(int n)
{
    return initialize_graph(n, false);
}

/**
Always inserts, never updates.
*/
void insert(Graph* g, int i, int j)
{
    Cell new_cell = (Cell) malloc(sizeof(cell));
    new_cell->parent = i;
    new_cell->v = j;

    // Push front the new cell to g[i]
    new_cell->next = (*g)[i];
    (*g)[i] = new_cell;
}

/**
Avoid to manipulate pointers. Its better to deal with values.
*/
void insert(Graph* g, Cell node_info)
{
    int i = node_info->parent;
    int j = node_info->v;

    insert(g, i, j);
}

bool exists_edge(Cell linked_list, int target)
{
    for (Cell cell = linked_list; cell != NULL; cell = cell->next)
    {
        if (cell->v == target)
        {
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
    for (Cell cell = linked_list; cell != NULL; cell = cell->next)
    {
        if (show_all_info)
        {
            cout << cell->v << "(" << cell->parent << ")  "; //
        }
        else
        {
            cout << cell->v << " ";
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
}

void print_graph(Graph g)
{
    print_graph(g, false);
}

void print_statistics(Graph g)
{
    // 1-based
    cout << "\nTotal nodes: " << g.size() - 1 << endl;

    int total_edges = 0;

    for (int i = 1; i < g.size(); i++)
        for (Cell cell = g[i]; cell != NULL; cell = cell->next)
            total_edges++;

    cout << "Total edges: " << total_edges << endl;
}

void print_all_sccs(vector<SCC> sccs)
{
    for (int k = 0; k < sccs.size(); k++) {
        vector<int> all_nodes_scc = sccs[k].nodes;
        cout << "SCC #" << k << ": Type = " << sccs[k].type << ", ";
        cout << "Total of nodes = " << all_nodes_scc.size() << ", " << endl;
        cout << "Nodes = [";
        for (int l = 0; l < all_nodes_scc.size(); l++)
            cout << all_nodes_scc[l] << ", ";
        cout << "] " << endl;
    }
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

        if (left[iLeft]->v > right[iRight]->v)
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

Graph calculate_transpose(Graph g)
{
    Graph gt = initialize_graph(g.size());

    for (int i = 1; i < g.size(); i++)
        for (Cell cell = g[i]; cell != NULL; cell = cell->next)
            insert(&gt, cell->v, i);

    return gt;
}

vector<int> inverse(vector<int> x)
{
    vector<int> inverted(x.size());

    for (int i = 1; i < x.size(); i++)
        inverted[x[i]] = i;

    return inverted;
}

void visit(Graph g, int source, vector<bool>* visited, vector<int>* finish_times, int* time)
{
    (*visited)[source] = true;

    for (int j = 1; j < g.size(); j++)
    {
        if (exists_edge(g, source, j) && (*visited)[j] == false)
        {
            visit(g, j, visited, finish_times, time);
        }
    }

    (*finish_times)[source] = (*time)++;
}

vector<int> dfs(Graph g)
{
    vector<bool> visited(g.size());
    vector<int> finish_times(g.size());
    int time = 1;

    for (int i = 1; i < g.size(); i++) {
        if (!visited[i]) {
            visit(g, i, &visited, &finish_times, &time);
        }
    }

    return finish_times;
}

vector<int> single_dfs(Graph g, int root)
{
    vector<bool> visited(g.size());
    vector<int> finish_times(g.size());
    int time = 1;

    visit(g, root, &visited, &finish_times, &time);

    vector<int> visited_nodes;
    for (int i = 1; i < visited.size(); i++) {
        if (visited[i])
            visited_nodes.push_back(i);
    }

    return visited_nodes;
}

vector<SCC> calculate_sccs(Graph g, Graph gt)
{
    vector<SCC> all_sccs;

    // Nodes that will be excluded from next DFSs
    vector<int> excluded_nodes;

    vector<int> finish_times = dfs(g);

    // Considering that for every node i, 1 <= i <= |V|
    vector<int> sorted_nodes = inverse(finish_times);

    for (int i = sorted_nodes.size() - 1; i > 0; i--)
    {
        vector<int> visited_nodes = single_dfs(gt, i);

        SCC new_scc;
        new_scc.nodes = visited_nodes;
        new_scc.type = 1;
        all_sccs.push_back(new_scc);


    }

    return all_sccs;
}

int main(int argc, char** argv)
{
    cout.precision(3);

    // Input Reader
    string line;
    ifstream inputFile;

    // TOREMOVE
    if (argc == 1)
    {
        inputFile.open("input-tp2-specification.txt"); // web-Stanford.txt
    }
    else if (argc > 1)
    {
        inputFile.open(argv[1]);
    } else
    {
        cout << "Input file not specified." << endl;
        return 1;
    }

    if (!inputFile.is_open())
    {
        cout << "Problem reading input file." << endl;
        return 1;
    }

    // It will keep all the information of file
    vector<int> sources, targets;
    sources.resize(INITIAL_CAPACITY);
    targets.resize(INITIAL_CAPACITY);

    int i, j, greatest = 0;

    // Input File Reader
    while (getline(inputFile, line))
    {
        // skip all lines beginning with anything different from digits
        // useful for web graphs txts from Stanford Website
        if (isdigit(line[0]) == false)
        {
            continue;
        }

        stringstream stream(line);

        stream >> i >> j;

        sources.push_back(i);
        targets.push_back(j);

        // keep the greatest node id found
        if (i > greatest)
        {
            greatest = i;
        }
        if (j > greatest)
        {
            greatest = j;
        }
    }

    inputFile.close();


    // number of nodes
    int n = greatest;

    // 1-based: n + 1
    Graph g = initialize_graph(n + 1);

    for (int k = 0; k < sources.size(); k++)
    {
        insert(&g, sources[k], targets[k]);
    }

    Graph gt = calculate_transpose(g);

    vector<SCC> sccs = calculate_sccs(g, gt);

    print_all_sccs(sccs);

    // print_graph(g);
    print_statistics(g);

}
