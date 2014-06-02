#include "../../include/util/graph.h"

std::vector<double> dijkstra(Graph g, int v0)
{
    std::vector<double> distances(g.numVertices(), gr_infinity);
    std::priority_queue<vertexCost, std::vector<vertexCost>, cmpVertexDijkstra> Q;
    Q.push(vertexCost(v0, 0.0));
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


bool reachable(Graph g, int u, int v)
{
    if (u == v)
        return true;
    std::list<int> Q;
    Q.push_front(u);
    std::unordered_set<int> visited;
    while (!Q.empty()) {
        int tmp = Q.front();
        Q.pop_front();
        visited.insert(tmp);
        std::unordered_map<int,double> neighbors = g.neighbors(tmp);
        for (std::pair<int, double> x : neighbors) {
            if (visited.find(x.first) != visited.end())
                continue;
            if (x.first == v)
                return true;
            Q.push_front(x.first);
        }
    }
    return false;
}
