/****************************************************
*                                                   *
* License: Apache License 2.0                       *
* Author: Dario Ostuni <another.code.996@gmail.com> *
*                                                   *
****************************************************/

#pragma once

#include <cassert>
#include <memory>

namespace ostuni
{
template <typename T>
class bst
{
protected:
    template <typename U>
    class bst_node
    {
    public:
        U value;
        std::shared_ptr<bst_node<T>> left;
        std::shared_ptr<bst_node<T>> right;
        std::weak_ptr<bst_node<T>> parent;
        bst_node()
        {
            value = U();
            left.reset();
            right.reset();
            parent.reset();
        }
    };
    std::shared_ptr<bst_node<T>> root;
    std::shared_ptr<bst_node<T>> _search(const T& value, std::shared_ptr<bst_node<T>> node)
    {
        if(!node)
            return std::shared_ptr<bst_node<T>>();
        if(value == node->value)
            return node;
        if(value < node->value)
            return _search(value, node->left);
        return _search(value, node->right);
    }
    void _erase(std::shared_ptr<bst_node<T>> node)
    {
        if(!node->right && !node->left)
        {
            auto parent = node->parent.lock();
            if(parent->right && (parent->right->value == node->value))
                parent->right.reset();
            else
                parent->left.reset();
            return;
        }
        if(node->right)
        {
            auto tmp = node->right;
            while(tmp->left)
                tmp = tmp->left;
            swap(node->value, tmp->value);
            _erase(tmp);
            return;
        }
        else if(node->left)
        {
            auto tmp = node->left;
            while(tmp->right)
                tmp = tmp->right;
            swap(node->value, tmp->value);
            _erase(tmp);
            return;
        }
        else
        {
            assert(!!!"Something went wrong!");
        }
    }

public:
    bst()
    {
        root.reset();
    }
    bool erase(const T& value)
    {
        auto tmp = _search(value, root);
        if(!tmp)
            return false;
        if((tmp->value == root->value) && !root->right && !root->left)
        {
            root.reset();
            return true;
        }
        _erase(tmp);
        return true;
    }
    bool insert(const T& value)
    {
        std::shared_ptr<bst_node<T>> tmp = std::make_shared<bst_node<T>>();
        tmp->value = value;
        if(!root)
        {
            root = tmp;
            return true;
        }
        std::shared_ptr<bst_node<T>> last_node = root;
        while(true)
        {
            if(value > last_node->value)
            {
                if(last_node->right)
                {
                    last_node = last_node->right;
                }
                else
                {
                    last_node->right = tmp;
                    tmp->parent = last_node;
                    return true;
                }
            }
            else if(value < last_node->value)
            {
                if(last_node->left)
                {
                    last_node = last_node->left;
                }
                else
                {
                    last_node->left = tmp;
                    tmp->parent = last_node;
                    return true;
                }
            }
            else
            {
                return false;
            }
        }
    }
    bool find(const T& value)
    {
        return bool(_search(value, root));
    }
};
}
