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
#include <map>

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
    int id;

    vector<int> nodes;

    int type;
};

typedef struct cell* Cell;

typedef vector<Cell> Graph;

void initialize_graph(Graph* graph, int n, bool randomized)
{
    (*graph).resize(n);
    for (int i = 1; i < n; i++)
    {
        (*graph)[i] = NULL;
    }
}

void initialize_graph(Graph* graph, int n)
{
    return initialize_graph(graph, n, false);
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
    // FIXME if linked list is badly allocated, this loop possibly never ends
    for (Cell cell = linked_list; cell != NULL; cell = cell->next)
    {
        if (cell->v == target)
        {
            return true;
        }
    }

    return false;
}

bool exists_edge(Graph* g, int source, int target)
{
    return exists_edge((*g)[source], target);
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

void print_graph(Graph* g, bool show_all_info)
{
    cout << endl;

    for (int i = 1; i < (*g).size(); i++)
    {
        cout << i << ": ";
        print_linked_list((*g)[i], show_all_info);
    }
}

void print_graph(Graph* g)
{
    print_graph(g, false);
}

void print_statistics(Graph* g)
{
    // 1-based
    cout << "\nTotal of Nodes: " << (*g).size() - 1 << endl;

    int total_edges = 0;

    for (int i = 1; i < (*g).size(); i++)
        for (Cell cell = (*g)[i]; cell != NULL; cell = cell->next)
            total_edges++;

    cout << "Total of Edges: " << total_edges << endl;
}

void print_all_sccs(vector<SCC>* sccs)
{
    cout << "Total of SCCs: " << (*sccs).size() << endl << endl;

    for (int k = 0; k < (*sccs).size(); k++) {
        cout << "SCC Adr: " << &((*sccs)[k]) << endl;

        vector<int> all_nodes = (*sccs)[k].nodes;

        cout << "SCC #" << (*sccs)[k].id << ": Type = " << (*sccs)[k].type << ", ";
        cout << "Total of Nodes = " << all_nodes.size() << ", " << endl;
        cout << "Nodes = [ ";
        for (int l = 0; l < all_nodes.size(); l++)
            cout << all_nodes[l] << " ";
        cout << "]\n\n";
    }
}

void calculate_transpose(Graph* g, Graph* gt)
{
    initialize_graph(gt, (*g).size());

    for (int i = 1; i < (*g).size(); i++)
        for (Cell cell = (*g)[i]; cell != NULL; cell = cell->next)
            insert(gt, cell->v, i);
}

void inverse(vector<int>* original, vector<int>* inverted)
{
    (*inverted).resize((*original).size());

    for (int i = 1; i < (*original).size(); i++)
    {
        (*inverted)[(*original)[i]] = i;
    }
}

void visit(Graph* g, int source, vector<bool>* visited, vector<int>* finish_times, int* time, vector<bool>* ignored_nodes)
{
    // cout << "  trying to visit from source = " << source << endl;
    // if the node was already visited or if it must be ignored, do nothing
    if ((*visited)[source] || (*ignored_nodes)[source])
    {
        return;
    }

    // cout << "  new node! I will do dfs from = " << source << endl;
    // flag the node as visited
    (*visited)[source] = true;

    // if node has no children, increments time and return
    if ((*g)[source] == NULL)
    {
        (*finish_times)[source] = (*time)++;
        return;
    }

    // if the node has children, visit them recursively
    for (Cell cell = (*g)[source]; cell != NULL; cell = cell->next)
    {
        visit(g, cell->v, visited, finish_times, time, ignored_nodes);
        // cout << "    trying to visit from source = " << source << " and target = " << cell->v << endl;
        // if ((*visited)[cell->v] == false && !ignored_nodes[cell->v])
        // {
        // }
    }

    (*finish_times)[source] = (*time)++;
}

void dfs(Graph* g, vector<bool>* ignored_nodes, vector<int>* finish_times)
{
    vector<bool> visited((*g).size());
    (*finish_times).resize((*g).size());

    int time = 1;

    for (int i = 1; i < (*g).size(); i++) {

        if ((*g)[i] == NULL || visited[i] || (*ignored_nodes)[i])
        {
            continue;
        }

        // cout << "main dfs for i = " << i << endl;
        visit(g, i, &visited, finish_times, &time, ignored_nodes);
    }

}

// Calls visit
void single_dfs(Graph* g, int root, vector<bool>* ignored_nodes, vector<int>* visited_nodes)
{
    vector<bool> all_visited((*g).size());
    vector<int> finish_times((*g).size());
    int time = 1;

    visit(g, root, &all_visited, &finish_times, &time, ignored_nodes);

    // Copy from all nodes only the visited nodes to a single vector
    for (int i = 1; i < all_visited.size(); i++) {
        if (all_visited[i] && !(*ignored_nodes)[i]) // ignored_nodes need | dont think so
            (*visited_nodes).push_back(i);
    }
}

void calculate_sccs(Graph* g, Graph* gt, vector<SCC>* all_sccs, vector<SCC>* nodes_sccs)
{
    int largest_scc_size = 0;
    int g_size = (*g).size();

    // Nodes that will be ignored from next DFSs
    vector<bool> ignored_nodes(g_size);

    // Vector that maps the nodes to theirs SCCs
    (*nodes_sccs).resize(g_size);

    cout << "Starting first dfs..." << endl;
    vector<int> finish_times;
    dfs(g, &ignored_nodes, &finish_times);
    cout << "First dfs ok." << endl;

    // Considering that for every node i, 1 <= i <= |V|
    vector<int> sorted_nodes;
    inverse(&finish_times, &sorted_nodes);

    for (int i = sorted_nodes.size() - 1; i > 0; i--)
    {
        int root = sorted_nodes[i];

        if (root == 0 || ignored_nodes[root])
        {
            continue;
        }

        vector<int> visited_nodes;
        single_dfs(gt, root, &ignored_nodes, &visited_nodes);

        SCC new_scc;
        new_scc.id = root;
        new_scc.nodes = visited_nodes;
        new_scc.type = 0;
        (*nodes_sccs)[root] = new_scc;

        (*all_sccs).push_back(new_scc);

        // cout << "New SCC: #" << root << endl;

        for (int k = 0; k < visited_nodes.size(); k++)
        {
            int already_visited = visited_nodes[k];
            ignored_nodes[already_visited] = true;
            (*nodes_sccs)[already_visited] = new_scc;

            // cout << "  Node " << already_visited << " is at SCC #" << root << endl; // as imprime pro root as vezes não o.O
            // cout << (*nodes_sccs)[already_visited].id << endl;
        }

        if (visited_nodes.size() > largest_scc_size)
        {
            largest_scc_size = visited_nodes.size();
            cout << "Found a new bigger SCC! Id: #" << new_scc.id << ", ";
            cout << "Size: " << largest_scc_size << endl;
        }
    }
}

// Just map all the sccs ids to numbers in [0, ..., sccs.size]
void populate_map_ids(vector<SCC>* sccs, map<int,int>* map_ids)
{
    for (int i = 0; i < (*sccs).size(); i++)
    {
        int scc_id = (*sccs)[i].id;
        // 1-based
        (*map_ids)[scc_id] = i + 1;
        // cout << "maps " << scc_id << ": " << i + 1 << endl;
    }
}

// ? Assume that every scc has at least one node
void compress_graph(Graph* g, vector<SCC>* sccs, vector<SCC>* nodes_sccs, map<int,int>* map_ids, Graph* compressed_graph)
{
    // 1-based
    initialize_graph(compressed_graph, (*sccs).size() + 1);

    populate_map_ids(sccs, map_ids);

    for (int i = 1; i < (*g).size(); i++)
        for (Cell cell = (*g)[i]; cell != NULL; cell = cell->next)
        {
            // if different groups, insert edge from Source SCC -> Target SCC
            int sourceSCCId = (*map_ids)[(*nodes_sccs)[i].id];
            int targetSCCid = (*map_ids)[(*nodes_sccs)[cell->v].id];

            // cout << "Analysing SCC edge " << sourceSCCId << " -> " << targetSCCid << endl;

            if (sourceSCCId != targetSCCid && !exists_edge(compressed_graph, sourceSCCId, targetSCCid))
            {
                // cout << "  Inserting SCC edge " << sourceSCCId << " -> " << targetSCCid << endl;
                insert(compressed_graph, sourceSCCId, targetSCCid);
            }

            // cout << "  End analysing SCC edge " << sourceSCCId << " -> " << targetSCCid << endl;
        }
}

int main(int argc, char** argv)
{
    // Input Reader
    string line;
    ifstream inputFile;

    cout << "Start reading..." << endl;

    // TOREMOVE
    if (argc == 1)
    {
        inputFile.open("input-tp2-specification.txt"); // web-Stanford.txt  my-tests.txt
    }                                   // input-tp2-specification.txt
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
    Graph g;
    initialize_graph(&g, n + 1);

    for (int k = 0; k < sources.size(); k++)
    {
        insert(&g, sources[k], targets[k]);
    }

    Graph gt;
    calculate_transpose(&g, &gt);

    cout << "Start discovering SCCs..." << endl;

    vector<SCC> sccs;
    vector<SCC> nodes_sccs;
    calculate_sccs(&g, &gt, &sccs, &nodes_sccs);

    cout << "Start compressing of original graph..." << endl;

    Graph compressed_graph;
    map <int, int> map_scc_ids;//   (sccs.size());
    compress_graph(&g, &sccs, &nodes_sccs, &map_scc_ids, &compressed_graph);

    // print_all_sccs(&sccs);
    // print_graph(&compressed_graph);
    // print_graph(&g);
    print_statistics(&g);
    print_statistics(&compressed_graph);
}
