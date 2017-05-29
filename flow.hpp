/*
 * Author: Dario Ostuni <dario.ostuni@gmail.com>
 * License: Apache 2.0
 *
 * Time complexity (with policy::max_label): O(V²log(V)√E)
 * Time complexity (with policy::fifo): O(V³)
*/

#pragma once

#include <vector>
#include <tuple>
#include <cassert>
#include <unordered_map>
#include <algorithm>
#include <climits>
#include <queue>

namespace ostuni {

namespace policy {

class max_label
{
protected:
    class node
    {
    public:
        size_t index;
        size_t label;
        bool operator >(const node& o) const
        {
            return this->label > o.label;
        }
        node(size_t in, size_t la)
        {
            index = in;
            label = la;
        }
    };
    std::priority_queue<node, std::vector<node>, std::greater<node>> pq;
public:
    void add(size_t index, size_t label)
    {
        pq.push(node(index, label));
    }
    size_t next()
    {
        size_t top = pq.top().index;
        pq.pop();
        return top;
    }
    bool empty()
    {
        return pq.empty();
    }
};

class fifo
{
protected:
    std::queue<size_t> q;
public:
    void add(size_t index, size_t label)
    {
        (void)label;
        q.push(index);
    }
    size_t next()
    {
        size_t front = q.front();
        q.pop();
        return front;
    }
    bool empty()
    {
        return q.empty();
    }
};

}

template<typename T, typename U = policy::fifo>
static T flow(size_t n, const std::vector<std::tuple<size_t, size_t, T>>& edges, size_t source, size_t sink)
{
    using namespace std;
    vector<unordered_map<size_t, T>> graph(n);
    T upper_bound = T(0);
    for(const auto& i: edges)
    {
        graph[get<0>(i)][get<1>(i)] += get<2>(i);
        if(get<0>(i) == source)
            upper_bound += get<2>(i);
    }
    vector<size_t> height(n);
    vector<T> excess(n, T(0));
    excess[source] = upper_bound;
    U node_queue;
    auto relabel = [&graph, &height](size_t node) -> void
    {
        size_t min_height = ULONG_MAX;
        for(const auto& edge: graph[node])
        {
            min_height = min(min_height, height[edge.first]);
            height[node] = min_height + 1;
        }
    };
    auto push = [&graph, &excess](size_t node, size_t pushed)
    {
        T quantity = min(excess[node], graph[node][pushed]);
        excess[node] -= quantity;
        excess[pushed] += quantity;
        graph[pushed][node] += quantity;
        graph[node][pushed] -= quantity;
        if(graph[node][pushed] == 0)
            graph[node].erase(pushed);
    };
    auto discharge = [&graph, &n, &push, &relabel, &height, &excess, &node_queue, &sink, &source](size_t node)
    {
        while(excess[node] > T(0))
        {
            for(size_t i = 0; i < n && excess[node] > T(0); i++)
            {
                if(graph[node].count(i) && (height[node] == height[i] + 1 || node == source))
                {
                    push(node, i);
                    if(i != sink && i != source)
                        node_queue.add(i, height[i]);
                }
            }
            if(excess[node] > T(0))
            {
                relabel(node);
            }
        }
    };
    height[source] = n;
    node_queue.add(source, height[source]);
    while(!node_queue.empty())
    {
        size_t node = node_queue.next();
        discharge(node);
    }
    return excess[sink];
}

}
