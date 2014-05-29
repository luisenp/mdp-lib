#ifndef MDPLIB_GRAPH_H
#define MDPLIB_GRAPH_H

#include <vector>
#include <unordered_set>
#include <queue>
#include <limits>
#include <cassert>

#define vc_vertex first
#define vc_cost second

const double gr_inf = std::numeric_limits<double>::max();

typedef std::pair<int, double> vertexCost;

class Graph
{
private:
    std::vector< std::unordered_map<int, double> > adjList;
public:
    Graph(int numVertices)
    {
        for (int i = 0; i < numVertices; i++) {
            adjList.push_back(std::unordered_map<int, double> () );
        }
    }

    double weight(int i, int j)
    {
        assert(i < adjList.size() && j < adjList.size());
        if (adjList[i].find(j) == adjList[i].end())
            return gr_inf;
        else return adjList[i][j];
    }

    bool connect(int i, int j, double weight)
    {
        assert(i < adjList.size() && j < adjList.size());
        adjList[i][j] = weight;
    }

    std::unordered_map<int, double> neighbors(int i)
    {
        assert(i < adjList.size());
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
std::vector<double> dijkstra(Graph g, int v0)
{
    std::vector<double> distances(g.numVertices(), gr_inf);
    std::priority_queue<vertexCost, std::vector<vertexCost>, cmpVertexDijkstra> Q;
    Q.push(vertexCost(0, 0.0));
    std::unordered_set<int> closed;
    while (!Q.empty()) {
        vertexCost uc = Q.top();
        Q.pop();
        distances[uc.vc_vertex] = std::min(uc.vc_cost, distances[uc.vc_vertex]);
        closed.insert(uc.vc_vertex);
        std::unordered_map<int,double> neighbors = g.neighbors(uc.vc_vertex);
        for (std::pair<int,double> vc : neighbors) {
            if (closed.find(vc.first) != closed.end())
                continue;
            Q.push(vertexCost(vc.first, uc.vc_cost + vc.second));
        }
    }
    return distances;
}

#endif // MDPLIB_GRAPH_H
