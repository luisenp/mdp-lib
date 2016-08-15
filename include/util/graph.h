#ifndef MDPLIB_GRAPH_H
#define MDPLIB_GRAPH_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <list>
#include <limits>
#include <cassert>

#define vc_vertex first
#define vc_cost second

const double gr_infinity = std::numeric_limits<double>::max();

typedef std::pair<int, double> vertexCost;

class Graph
{
private:
    std::vector< std::unordered_map<int, double> > adjList;

public:
    Graph() {}

    Graph(int numVertices)
    {
        for (int i = 0; i < numVertices; i++) {
            adjList.push_back(std::unordered_map<int, double> () );
        }
    }

    Graph(Graph& g)
    {
        adjList = g.adjList;
    }

    Graph& operator=(const Graph& rhs)
    {
        if (this == &rhs)
            return *this;

        adjList =  rhs.adjList;
        return *this;

    }

    double weight(unsigned int i, unsigned int j)
    {
        assert(i >= 0 && i < adjList.size() && j >= 0 && j < adjList.size());
        if (adjList[i].find(j) == adjList[i].end())
            return gr_infinity;
        else return adjList[i][j];
    }

    bool connect(unsigned int i, unsigned int j, double weight)
    {
        assert(i >= 0 && i < adjList.size() && j >= 0 && j < adjList.size());
        adjList[i][j] = weight;
    }

    std::unordered_map<int, double>& neighbors(unsigned int i)
    {
        assert(i >= 0 && i < adjList.size());
        return adjList[i];
    }

    int numVertices()
    {
        return adjList.size();
    }
};

class cmpVertexDijkstra
{
public:
    bool operator() (const vertexCost& lhs, const vertexCost&rhs) const
    {
        return (lhs.vc_cost > rhs.vc_cost);
    }
};

/**
 * Returns the single source shortest distances from the given vertex to all
 * vertices on the given graph.
 */
std::vector<double> dijkstra(Graph g, int v0);

/**
 * Checks if vertex v can be reached from vertex u in the given graph.
 */
 bool reachable(Graph g, int u, int v);

#endif // MDPLIB_GRAPH_H
