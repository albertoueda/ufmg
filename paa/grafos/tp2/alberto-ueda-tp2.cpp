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

    /*
    Types of SCC:
    1 = largest SCC
    2 = IN
    3 = OUT
    4 = TENDRIL IN
    5 = TENDRIL OUT
    6 = TUBE
    7 = DISCONNECTED
    */
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
    cout << "\n\nTotal of Nodes: " << (*g).size() - 1 << endl;

    int total_edges = 0;

    for (int i = 1; i < (*g).size(); i++)
        for (Cell cell = (*g)[i]; cell != NULL; cell = cell->next)
            total_edges++;

    cout << "Total of Edges: " << total_edges << endl;
}

void print_all_sccs(vector<SCC>* sccs)
{
    cout << "Total of SCCs found: " << (*sccs).size() << endl << endl;

    for (int type = 1; type < 8; type++)
    {
        for (int k = 0; k < (*sccs).size(); k++)
        {
            if ((*sccs)[k].type != type)
                continue;

            vector<int> all_nodes = (*sccs)[k].nodes;

            cout << "SCC #" << (*sccs)[k].id << ": Type = " << (*sccs)[k].type << ", ";
            cout << "Total of Nodes = " << all_nodes.size() << ", ";
            cout << "Nodes = [ ";
            for (int l = 0; l < all_nodes.size(); l++)
                cout << all_nodes[l] << " ";
            cout << "]\n";
        }
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

void visit(Graph* g, int source, vector<bool>* visited, vector<int>* finish_times, int* time,
           vector<bool>* ignored_nodes)
{
    if ((*visited)[source] || (*ignored_nodes)[source])
    {
        return;
    }

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

        visit(g, i, &visited, finish_times, &time, ignored_nodes);
    }

}

// Calls visit
void single_dfs(Graph* g, int root, vector<bool>* ignored_nodes, vector<int>* visited_nodes, bool ignore_root)
{
    vector<bool> all_visited((*g).size());
    vector<int> finish_times((*g).size());
    int time = 1;

    visit(g, root, &all_visited, &finish_times, &time, ignored_nodes);

    // Copy from all nodes only the visited nodes to a single vector
    for (int i = 1; i < all_visited.size(); i++)
    {
        if (ignore_root && i == root)
            continue;

        if (all_visited[i] && !(*ignored_nodes)[i])
            (*visited_nodes).push_back(i);
    }
}

// Calls visit
void single_dfs(Graph* g, int root, vector<bool>* ignored_nodes, vector<int>* visited_nodes)
{
    single_dfs(g, root, ignored_nodes, visited_nodes, false);
}

int calculate_sccs(Graph* g, Graph* gt, vector<SCC>* all_sccs, vector<SCC>* nodes_sccs)
{
    int largest_scc_size = 0;
    int largest_scc_id = 0;
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
        new_scc.type = 7;
        (*nodes_sccs)[root] = new_scc;

        (*all_sccs).push_back(new_scc);

        for (int k = 0; k < visited_nodes.size(); k++)
        {
            int already_visited = visited_nodes[k];
            ignored_nodes[already_visited] = true;
            (*nodes_sccs)[already_visited] = new_scc;
        }

        if (visited_nodes.size() > largest_scc_size)
        {
            largest_scc_size = visited_nodes.size();
            largest_scc_id = new_scc.id;
            cout << "Found a new bigger SCC! Id: #" << new_scc.id << ", ";
            cout << "Size: " << largest_scc_size << endl;
        }
    }

    // Update the type of the largest SCC to 1
    for (int i = 0; i < (*all_sccs).size(); i++)
    {
        if ((*all_sccs)[i].id == largest_scc_id)
        {
            (*all_sccs)[i].type = 1;
            break;
        }
    }

    return largest_scc_id;
}

// Just map all the sccs ids to numbers in [0, ..., sccs.size]
void populate_map_ids(vector<SCC>* sccs, map<int,int>* map_ids)
{
    for (int i = 0; i < (*sccs).size(); i++)
    {
        int scc_id = (*sccs)[i].id;

        // 1-based
        (*map_ids)[scc_id] = i + 1;
    }
}

void compress_graph(Graph* g, vector<SCC>* sccs, vector<SCC>* nodes_sccs,
                    map<int,int>* map_ids, Graph* compressed_graph)
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

            if (sourceSCCId != targetSCCid && !exists_edge(compressed_graph, sourceSCCId, targetSCCid))
            {
                insert(compressed_graph, sourceSCCId, targetSCCid);
            }
        }
}

void inverse_map(map<int, int>* original, map<int, int>* reversed)
{
    for(map<int, int>::const_iterator it = (*original).begin(); it != (*original).end(); ++it )
    {
        (*reversed)[it->second] = it->first;
    }
}

void find_sccs_out(Graph* compressed_graph, int largest_scc_id, vector<SCC>* sccs,
                   map<int, int>* map_sccs, map<int, int>* map_scc_reverse, vector<int>* visited_nodes)
{
    vector<bool> ignored_nodes((*compressed_graph).size());
    single_dfs(compressed_graph, largest_scc_id, &ignored_nodes, visited_nodes, true);

    for (int i = 0; i < (*visited_nodes).size(); i++) {
        int index_out_scc = (*map_sccs)[(*map_scc_reverse)[(*visited_nodes)[i]]];
        (*sccs)[index_out_scc].type = 3;
    }
}

void find_sccs_in(Graph* compressed_graph_t, int largest_scc_id, vector<SCC>* sccs,
                  map<int, int>* map_sccs, map<int, int>* map_scc_reverse,
                  vector<int>* visited_nodes)
{
    vector<bool> ignored_nodes((*compressed_graph_t).size());
    single_dfs(compressed_graph_t, largest_scc_id, &ignored_nodes, visited_nodes, true);

    for (int i = 0; i < (*visited_nodes).size(); i++) {
        int index_in_scc = (*map_sccs)[(*map_scc_reverse)[(*visited_nodes)[i]]];
        (*sccs)[index_in_scc].type = 2;
    }
}

void find_sccs_in_tendrils(Graph* compressed_graph, int largest_scc_id, vector<int>* in_sccs, vector<int>* out_sccs,
                           vector<SCC>* sccs, map<int, int>* map_sccs, map<int, int>* map_scc_reverse,
                           vector<int>* visited_nodes)
{
    vector<bool> ignored_nodes((*compressed_graph).size());
    ignored_nodes[largest_scc_id] = true;

    // Ignore also all the OUT nodes
    for (int k = 0; k < (*out_sccs).size(); k++)
    {
        ignored_nodes[(*out_sccs)[k]] = true;
    }

    for (int k = 0; k < (*in_sccs).size(); k++)
    {
        single_dfs(compressed_graph, (*in_sccs)[k], &ignored_nodes, visited_nodes, true);
        for (int i = 0; i < (*visited_nodes).size(); i++) {
            ignored_nodes[(*visited_nodes)[i]] = true;
        }
    }

    for (int i = 0; i < (*visited_nodes).size(); i++) {
        int index_scc = (*map_sccs)[(*map_scc_reverse)[(*visited_nodes)[i]]];

        int scc_type = (*sccs)[index_scc].type;

        // If is also an out tendril, it is a tube
        if (scc_type == 5)
            (*sccs)[index_scc].type = 6;
        else if (scc_type == 7)
            (*sccs)[index_scc].type = 4;
    }
}

void find_sccs_out_tendrils(Graph* compressed_graph_t, int largest_scc_id, vector<int>* out_sccs, vector<int>* in_sccs,
                            vector<SCC>* sccs, map<int, int>* map_sccs, map<int, int>* map_scc_reverse,
                            vector<int>* visited_nodes)
{
    vector<bool> ignored_nodes((*compressed_graph_t).size());
    ignored_nodes[largest_scc_id] = true;

    // Ignore also all the IN nodes
    for (int k = 0; k < (*in_sccs).size(); k++)
    {
        ignored_nodes[(*in_sccs)[k]] = true;
    }

    for (int k = 0; k < (*out_sccs).size(); k++)
    {
        single_dfs(compressed_graph_t, (*out_sccs)[k], &ignored_nodes, visited_nodes, true);
        for (int i = 0; i < (*visited_nodes).size(); i++) {
            ignored_nodes[(*visited_nodes)[i]] = true;
        }
    }

    for (int i = 0; i < (*visited_nodes).size(); i++) {
        int index_scc = (*map_sccs)[(*map_scc_reverse)[(*visited_nodes)[i]]];

        int scc_type = (*sccs)[index_scc].type;

        // If is also an in tendril, it is a tube
        if (scc_type == 4)
            (*sccs)[index_scc].type = 6;
        else if (scc_type == 7)
            (*sccs)[index_scc].type = 5;
    }
}

void undirect_graph(Graph* g, Graph* gt)
{
    // Inserts at g all the edges of gt
    for (int i = 1; i < (*gt).size(); i++)
        for (Cell cell = (*gt)[i]; cell != NULL; cell = cell->next)
            insert(g, i, cell->v);
}

void find_all_sccs_connected(Graph* compressed_graph, int largest_scc_id, vector<int>* all_connected_sccs)
{
    vector<bool> ignored_nodes((*compressed_graph).size());
    single_dfs(compressed_graph, largest_scc_id, &ignored_nodes, all_connected_sccs);
}

void create_scc_map(vector<SCC>* sccs, map<int, int>* map_sccs)
{
    for (int i = 0; i < (*sccs).size(); i++)
    {
        (*map_sccs)[(*sccs)[i].id] = i;
    }
}

vector<int> merge(vector<int> left, vector<int> right)
{
    int i = 0, iLeft = 0, iRight = 0;
    int size = left.size() + right.size();
    vector<int> sorted(size);

    while (i < size)
    {
        if (iLeft == left.size() || iRight == right.size())
        {
            break;
        }

        if (left[iLeft] < right[iRight])
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

vector<int> mergesort(vector<int> original, int p, int q)
{
    if (p < q)
    {
        int r = (p + q) / 2;
        vector<int> left = mergesort(original, p, r);
        vector<int> right = mergesort(original, r + 1, q);

        return merge(left, right);
    }

    // p == q. Just creates a vector of one element
    vector<int> single_vector(1);
    single_vector[0] = original[p];

    return single_vector;
}

void group_nodes_by_type(vector<SCC>* sccs, vector<vector<int> >* all_nodes_by_type)
{
    for (int i = 0; i < (*sccs).size(); i++)
    {
        vector<int>* nodes = &(*all_nodes_by_type)[(*sccs)[i].type];
        (*nodes).insert((*nodes).end(), (*sccs)[i].nodes.begin(), (*sccs)[i].nodes.end());
    }

    // Sort the nodes
    for (int i = 1; i < (*all_nodes_by_type).size(); i++)
    {
        (*all_nodes_by_type)[i] = mergesort((*all_nodes_by_type)[i], 0, (*all_nodes_by_type)[i].size() - 1);
    }
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

void print_files(vector<vector<int> >* all_nodes)
{
    cout << "Writing scc.txt..." << endl;
    print_nodes("scc.txt", &((*all_nodes)[1]));

    cout << "Writing in.txt..." << endl;
    print_nodes("in.txt", &((*all_nodes)[2]));

    cout << "Writing out.txt..." << endl;
    print_nodes("out.txt", &((*all_nodes)[3]));

    cout << "Writing tendrils_a.txt..." << endl;
    print_nodes("tendrils_a.txt", &((*all_nodes)[4]));

    cout << "Writing tendrils_b.txt..." << endl;
    print_nodes("tendrils_b.txt", &((*all_nodes)[5]));

    cout << "Writing tendrils_c.txt..." << endl;
    print_nodes("tendrils_c.txt", &((*all_nodes)[6]));

    cout << "Writing disconnected.txt..." << endl;
    print_nodes("disconnected.txt", &((*all_nodes)[7]));

}

int main(int argc, char** argv)
{
    // Input Reader
    string line;
    ifstream inputFile;

    cout << "Start reading..." << endl;

    if (argc == 1)
    {
        inputFile.open("input-tp2-specification.txt");
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
    sources.reserve(INITIAL_CAPACITY);
    targets.reserve(INITIAL_CAPACITY);

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
    int largest_scc_id = calculate_sccs(&g, &gt, &sccs, &nodes_sccs);

    cout << "Start compressing of original graph..." << endl;
    Graph compressed_graph;
    map <int, int> map_scc_ids;
    compress_graph(&g, &sccs, &nodes_sccs, &map_scc_ids, &compressed_graph);

    // Largest SCC id in the compressed version
    int compressed_largest_scc = map_scc_ids[largest_scc_id];

    // Given a id in the compressed graph, maps it to original SCC id
    map <int, int> map_scc_reverse;
    inverse_map(&map_scc_ids, &map_scc_reverse);

    // Creates an auxiliar map id->index to improve performance in searching sccs
    map<int, int> scc_map;
    create_scc_map(&sccs, &scc_map);

    // Calculates OUT SCCs
    vector<int> out_sccs;
    find_sccs_out(&compressed_graph, compressed_largest_scc, &sccs, &scc_map, &map_scc_reverse, &out_sccs);

    // Calculates IN SCCs
    Graph compressed_graph_t;
    calculate_transpose(&compressed_graph, &compressed_graph_t);
    vector<int> in_sccs;
    find_sccs_in(&compressed_graph_t, compressed_largest_scc, &sccs, &scc_map, &map_scc_reverse, &in_sccs);

    // Now the direction of compressed graph is not needed anymore
    // Transform the compressed direct graph in an undirected graph, in order to find the tubes
    undirect_graph(&compressed_graph, &compressed_graph_t);

    // Calculates IN Tendrils
    vector<int> in_tendrils_sccs;
    find_sccs_in_tendrils(&compressed_graph, compressed_largest_scc, &in_sccs, &out_sccs,
                          &sccs, &scc_map, &map_scc_reverse, &in_tendrils_sccs);

    // Calculates OUT Tendrils
    vector<int> out_tendrils_sccs;
    find_sccs_out_tendrils(&compressed_graph, compressed_largest_scc, &out_sccs, &in_sccs,
                           &sccs, &scc_map, &map_scc_reverse, &out_tendrils_sccs);

    // Output the results in files
    // 7 types, 1-based
    vector<vector<int> > all_nodes_by_type(8);
    for (int k = 0; k < all_nodes_by_type.size(); k++)
        all_nodes_by_type[k].reserve(INITIAL_CAPACITY/100);

    group_nodes_by_type(&sccs, &all_nodes_by_type);
    print_files(&all_nodes_by_type);

    /*
    print_all_sccs(&sccs);
    print_graph(&compressed_graph);
    print_graph(&g);
    print_statistics(&g);
    print_statistics(&compressed_graph);
    */

    cout << "Program finished." << endl;

}
