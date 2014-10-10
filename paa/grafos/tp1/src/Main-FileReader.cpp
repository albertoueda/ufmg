#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>

# define EPSILON 0.00001

# define DISCONNECTED_GRAPH_QUALITY -1.000

using namespace std;

int main()
{
    // initialize random seed:
    srand (time(NULL));
    cout.precision(3);

    // Input Reader
    string line;
    ifstream inputFile;
    inputFile.open("test.in.moodle");

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
        Graph candidate_graph;

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

            insert(&g, i, j, f, d);
            insert(&g, j, i, f, d);
        }

        /*
        cout << "Read graph:" << endl;
        print_graph(g);
        */

        if (!connected_graph(g))
        {
            cout << DISCONNECTED_GRAPH_QUALITY << endl;
            // cout << "graph(" << n << ") -> best quality: -1.000" << endl;
            // cout << "**************************************************************" << endl;
            continue;
        }

        // Determine the best quality using a binary search
        while (abs(upper_bound - lower_bound) > EPSILON)
        {
            // calculates the next candidate for quality
            quality_candidate = (upper_bound + lower_bound) / 2;
            // cout << "quality candidate: " << quality_candidate << endl;

            double sum_weights = 0.0;

            recalculate_weights(&g, quality_candidate);

            candidate_graph = kruskal(g);

            /*
            cout << "G:";
            print_graph(g);
            cout << "MST:";
            print_graph(candidate_graph);
            */

            // add edges of MST and also edges that will increase the ratio
            // if  w > 0  =>  f - r*d > 0  =>  f > r*d   =>  f/d > r
            for (int i = 1; i < candidate_graph.size(); i++)
            {
                for (Cell cell = g[i]; cell != NULL; cell = cell->next)
                {
                    if (exists_edge(candidate_graph, i, cell->v, cell->weight))
                    {
                        // cout << "Added: " << i << "->" << cell->v << endl;
                        sum_weights += cell->weight;
                    }
                    else if (i < cell->v && cell->weight > 0)
                    {
                        // cout << "Added: " << i << "->" << cell->v << endl;
                        sum_weights += cell->weight;

                        // For visualization of solution graph
                        insert(&candidate_graph, cell);
                    }
                }
            }

            /*
            cout << "sum of weights: " << sum_weights << endl;
            cout << "-------------------------------" << endl;
            */

            // stop if the sum of weights is good enough
            if (abs(sum_weights) <= EPSILON)
            {
                // cout << "sum_weights < " << EPSILON << "!\n\n";
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

        // best_quality = quality_candidate;

        if (abs(upper_bound - lower_bound) < EPSILON) {
            // cout << "upper_bound - lower_bound < " << EPSILON << "!\n\n";
        }

        /*
        cout << "G:";
        print_graph(g);
        cout << "last candidate:";
        print_graph(candidate_graph);
        */

        // Output Quality
        cout << fixed << quality_candidate << endl;
        // cout << "graph(" << n << ") -> best quality: " << quality_candidate << endl;
        // cout << "**************************************************************" << endl;
    /*
    */
    }

    inputFile.close();

}

