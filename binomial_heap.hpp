/****************************************************
*                                                   *
* License: Apache License 2.0                       *
* Author: Dario Ostuni <another.code.996@gmail.com> *
*                                                   *
****************************************************/

#pragma once

#include <cassert>
#include <memory>
#include <vector>

namespace ostuni
{
template <typename T>
class binomial_heap
{
protected:
    template <typename U>
    class binomial_heap_element;
    friend binomial_heap_element<T>;
    binomial_heap(const std::vector<std::shared_ptr<binomial_heap_element<T>>>& v)
    {
        nodes = v;
    }
    template <typename U>
    class binomial_heap_element
    {
    protected:
        std::vector<std::shared_ptr<binomial_heap_element<U>>> children;
        U value;
        size_t getDegree()
        {
            return children.size();
        }
        void attachElement(std::shared_ptr<binomial_heap_element<U>> e)
        {
            assert(getDegree() == e->getDegree());
            children.push_back(e);
        }
        binomial_heap<U> dismantle()
        {
            return binomial_heap<U>(children);
        }

    public:
        binomial_heap_element(const U& v)
        {
            value = v;
        }
        friend class binomial_heap<U>;
    };
    binomial_heap(const T& v)
    {
        nodes.push_back(std::make_shared<binomial_heap_element<T>>(v));
    }
    std::vector<std::shared_ptr<binomial_heap_element<T>>> nodes;
    long int getMinIndex()
    {
        long int min_index = -1;
        for(size_t i = 0; i < nodes.size(); i++)
        {
            if(nodes[i])
            {
                if(min_index < 0)
                    min_index = i;
                if(nodes[i]->value < nodes[min_index]->value)
                    min_index = i;
            }
        }
        return min_index;
    }

public:
    bool empty()
    {
        return !size();
    }
    void merge(binomial_heap& bh)
    {
        auto m = [](std::shared_ptr<binomial_heap_element<T>> e1, std::shared_ptr<binomial_heap_element<T>> e2) -> auto
        {
            assert(e1 && e2);
            if(e1->value < e2->value)
            {
                e1->attachElement(e2);
                return e1;
            }
            else
            {
                e2->attachElement(e1);
                return e2;
            }
        };
        std::shared_ptr<binomial_heap_element<T>> tmp;
        size_t i;
        for(i = 0; i < bh.nodes.size(); i++)
        {
            if(bh.nodes[i] || tmp)
            {
                if(nodes.size() <= i)
                {
                    nodes.resize(i + 1);
                }
                if(!nodes[i])
                {
                    if(bh.nodes[i] && tmp)
                    {
                        tmp = m(bh.nodes[i], tmp);
                    }
                    else if(bh.nodes[i])
                    {
                        nodes[i] = bh.nodes[i];
                    }
                    else
                    {
                        nodes[i] = tmp;
                        tmp.reset();
                    }
                }
                else
                {
                    if(bh.nodes[i] && tmp)
                    {
                        tmp = m(bh.nodes[i], tmp);
                    }
                    else if(bh.nodes[i])
                    {
                        tmp = m(bh.nodes[i], nodes[i]);
                        nodes[i].reset();
                    }
                    else
                    {
                        tmp = m(tmp, nodes[i]);
                        nodes[i].reset();
                    }
                }
            }
        }
        while(tmp)
        {
            if(nodes.size() <= i)
            {
                nodes.resize(i + 1);
            }
            if(!nodes[i])
            {
                nodes[i] = tmp;
                tmp.reset();
            }
            else
            {
                tmp = m(tmp, nodes[i]);
                nodes[i].reset();
            }
            i++;
        }
        bh.nodes.clear();
    }
    T top()
    {
        auto min_index = getMinIndex();
        assert(min_index >= 0);
        return nodes[min_index]->value;
    }
    void pop()
    {
        auto min_index = getMinIndex();
        assert(min_index >= 0);
        auto b_tmp = nodes[min_index]->dismantle();
        nodes[min_index].reset();
        merge(b_tmp);
    }
    void push(const T& x)
    {
        binomial_heap<T> b_tmp(x);
        merge(b_tmp);
    }
    size_t size()
    {
        size_t s = 0;
        for(size_t i = 0; i < nodes.size(); i++)
        {
            if(nodes[i])
                s += 1 << nodes[i]->getDegree();
        }
        return s;
    }
    binomial_heap()
    {
    }
};
}
