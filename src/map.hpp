/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
  * You can use sjtu::map as value_type by typedef.
    */
   typedef pair<const Key, T> value_type;

   // Forward declarations
   class const_iterator;
   class iterator;

  private:
   // AVL Tree Node
   struct Node {
       value_type *data;
       Node *left;
       Node *right;
       Node *parent;
       int height;

       Node() : data(nullptr), left(nullptr), right(nullptr), parent(nullptr), height(1) {}

       Node(const value_type &val) : left(nullptr), right(nullptr), parent(nullptr), height(1) {
           data = new value_type(val);
       }

       Node(const Node &other) : left(nullptr), right(nullptr), parent(nullptr), height(other.height) {
           if (other.data) {
               data = new value_type(*other.data);
           } else {
               data = nullptr;
           }
       }

       ~Node() {
           delete data;
       }

       void updateHeight() {
           int leftHeight = left ? left->height : 0;
           int rightHeight = right ? right->height : 0;
           height = (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
       }

       int balanceFactor() const {
           int leftHeight = left ? left->height : 0;
           int rightHeight = right ? right->height : 0;
           return leftHeight - rightHeight;
       }
   };

   Node *root;
   Node *endNode;
   size_t numElements;
   Compare comp;

   // Helper functions
   int getHeight(Node *node) const {
       return node ? node->height : 0;
   }

   void updateHeight(Node *node) {
       if (node) {
           node->updateHeight();
       }
   }

   int getBalance(Node *node) const {
       return node ? node->balanceFactor() : 0;
   }

   Node *rightRotate(Node *y) {
       Node *x = y->left;
       Node *T2 = x->right;

       x->right = y;
       y->left = T2;

       if (T2) T2->parent = y;
       x->parent = y->parent;
       y->parent = x;

       updateHeight(y);
       updateHeight(x);

       return x;
   }

   Node *leftRotate(Node *x) {
       Node *y = x->right;
       Node *T2 = y->left;

       y->left = x;
       x->right = T2;

       if (T2) T2->parent = x;
       y->parent = x->parent;
       x->parent = y;

       updateHeight(x);
       updateHeight(y);

       return y;
   }

   Node *balance(Node *node) {
       if (!node) return nullptr;

       updateHeight(node);

       int balance = getBalance(node);

       // Left Left Case
       if (balance > 1 && getBalance(node->left) >= 0) {
           return rightRotate(node);
       }

       // Left Right Case
       if (balance > 1 && getBalance(node->left) < 0) {
           node->left = leftRotate(node->left);
           if (node->left) node->left->parent = node;
           return rightRotate(node);
       }

       // Right Right Case
       if (balance < -1 && getBalance(node->right) <= 0) {
           return leftRotate(node);
       }

       // Right Left Case
       if (balance < -1 && getBalance(node->right) > 0) {
           node->right = rightRotate(node->right);
           if (node->right) node->right->parent = node;
           return leftRotate(node);
       }

       return node;
   }

   Node *findMin(Node *node) const {
       while (node && node->left) {
           node = node->left;
       }
       return node;
   }

   Node *findMax(Node *node) const {
       while (node && node->right) {
           node = node->right;
       }
       return node;
   }

   Node *insertNode(Node *node, Node *parent, const value_type &value, pair<iterator, bool> &result) {
       if (!node) {
           node = new Node(value);
           node->parent = parent;
           numElements++;
           result.first = iterator(node, this);
           result.second = true;
           return node;
       }

       if (comp(value.first, node->data->first)) {
           node->left = insertNode(node->left, node, value, result);
           if (node->left) node->left->parent = node;
       } else if (comp(node->data->first, value.first)) {
           node->right = insertNode(node->right, node, value, result);
           if (node->right) node->right->parent = node;
       } else {
           // Key already exists
           result.first = iterator(node, this);
           result.second = false;
           return node;
       }

       return balance(node);
   }

   Node *eraseNode(Node *node, const Key &key, bool &found) {
       if (!node) {
           found = false;
           return nullptr;
       }

       if (comp(key, node->data->first)) {
           node->left = eraseNode(node->left, key, found);
           if (node->left) node->left->parent = node;
       } else if (comp(node->data->first, key)) {
           node->right = eraseNode(node->right, key, found);
           if (node->right) node->right->parent = node;
       } else {
           // Found the node to delete
           found = true;
           numElements--;

           if (!node->left || !node->right) {
               Node *temp = node->left ? node->left : node->right;
               Node *parent = node->parent;

               if (!temp) {
                   temp = node;
                   node = nullptr;
               } else {
                   // Copy the child's data
                   delete node->data;
                   node->data = new value_type(*temp->data);
                   node->left = temp->left;
                   node->right = temp->right;
                   if (node->left) node->left->parent = node;
                   if (node->right) node->right->parent = node;
                   temp->left = nullptr;
                   temp->right = nullptr;
                   delete temp;
               }

               if (node) {
                   node->parent = parent;
               }
           } else {
               Node *successor = findMin(node->right);
               delete node->data;
               node->data = new value_type(*successor->data);

               bool dummy;
               node->right = eraseNode(node->right, successor->data->first, dummy);
               if (node->right) node->right->parent = node;
           }
       }

       if (!node) return nullptr;

       return balance(node);
   }

   Node *findNode(Node *node, const Key &key) const {
       while (node) {
           if (comp(key, node->data->first)) {
               node = node->left;
           } else if (comp(node->data->first, key)) {
               node = node->right;
           } else {
               return node;
           }
       }
       return nullptr;
   }

   void clearTree(Node *node) {
       if (node) {
           clearTree(node->left);
           clearTree(node->right);
           delete node;
       }
   }

   Node *copyTree(Node *otherNode, Node *parent) {
       if (!otherNode) return nullptr;
       Node *node = new Node(*otherNode);
       node->parent = parent;
       node->left = copyTree(otherNode->left, node);
       node->right = copyTree(otherNode->right, node);
       return node;
   }

  public:
   /**
  * see BidirectionalIterator at CppReference for help.
  *
  * if there is anything wrong throw invalid_iterator.
  *     like it = map.begin(); --it;
  *       or it = map.end(); ++end();
    */
   class iterator {
      private:
       Node *node;
       map *container;
       friend class const_iterator;
       friend class map;

       iterator(Node *n, map *c) : node(n), container(c) {}

      public:
       iterator() : node(nullptr), container(nullptr) {}

       iterator(const iterator &other) : node(other.node), container(other.container) {}

       iterator operator++(int) {
           if (!node) {
               throw invalid_iterator();
           }
           iterator temp = *this;
           ++(*this);
           return temp;
       }

       iterator &operator++() {
           if (!node) {
               throw invalid_iterator();
           }
           if (node->right) {
               node = container->findMin(node->right);
           } else {
               Node *parent = node->parent;
               while (parent && node == parent->right) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return *this;
       }

       iterator operator--(int) {
           iterator temp = *this;
           --(*this);
           return temp;
       }

       iterator &operator--() {
           if (!container || !container->root) {
               throw invalid_iterator();
           }
           if (!node) {
               node = container->findMax(container->root);
           } else if (node->left) {
               node = container->findMax(node->left);
           } else {
               Node *parent = node->parent;
               while (parent && node == parent->left) {
                   node = parent;
                   parent = parent->parent;
               }
               if (!parent) {
                   throw invalid_iterator();
               }
               node = parent;
           }
           return *this;
       }

       value_type &operator*() const {
           if (!node) {
               throw invalid_iterator();
           }
           return *node->data;
       }

       bool operator==(const iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       bool operator==(const const_iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       value_type *operator->() const noexcept {
           if (!node) {
               throw invalid_iterator();
           }
           return node->data;
       }
   };
   class const_iterator {
      private:
       Node *node;
       const map *container;
       friend class map;

       const_iterator(Node *n, const map *c) : node(n), container(c) {}

      public:
       const_iterator() : node(nullptr), container(nullptr) {}

       const_iterator(const const_iterator &other) : node(other.node), container(other.container) {}

       const_iterator(const iterator &other) : node(other.node), container(other.container) {}

       const_iterator operator++(int) {
           if (!node) {
               throw invalid_iterator();
           }
           const_iterator temp = *this;
           ++(*this);
           return temp;
       }

       const_iterator &operator++() {
           if (!node) {
               throw invalid_iterator();
           }
           if (node->right) {
               node = container->findMin(node->right);
           } else {
               Node *parent = node->parent;
               while (parent && node == parent->right) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return *this;
       }

       const_iterator operator--(int) {
           const_iterator temp = *this;
           --(*this);
           return temp;
       }

       const_iterator &operator--() {
           if (!container || !container->root) {
               throw invalid_iterator();
           }
           if (!node) {
               node = container->findMax(container->root);
           } else if (node->left) {
               node = container->findMax(node->left);
           } else {
               Node *parent = node->parent;
               while (parent && node == parent->left) {
                   node = parent;
                   parent = parent->parent;
               }
               if (!parent) {
                   throw invalid_iterator();
               }
               node = parent;
           }
           return *this;
       }

       const value_type &operator*() const {
           if (!node) {
               throw invalid_iterator();
           }
           return *node->data;
       }

       bool operator==(const iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       bool operator==(const const_iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       const value_type *operator->() const noexcept {
           if (!node) {
               throw invalid_iterator();
           }
           return node->data;
       }
   };

   map() : root(nullptr), endNode(nullptr), numElements(0) {}

   map(const map &other) : root(nullptr), endNode(nullptr), numElements(other.numElements) {
       root = copyTree(other.root, nullptr);
   }

   map &operator=(const map &other) {
       if (this != &other) {
           clear();
           numElements = other.numElements;
           root = copyTree(other.root, nullptr);
       }
       return *this;
   }

   ~map() {
       clear();
   }

   T &at(const Key &key) {
       Node *node = findNode(root, key);
       if (!node) {
           throw index_out_of_bound();
       }
       return node->data->second;
   }

   const T &at(const Key &key) const {
       Node *node = findNode(root, key);
       if (!node) {
           throw index_out_of_bound();
       }
       return node->data->second;
   }

   T &operator[](const Key &key) {
       Node *node = findNode(root, key);
       if (!node) {
           value_type val(key, T());
           pair<iterator, bool> result = insert(val);
           return result.first->second;
       }
       return node->data->second;
   }

   const T &operator[](const Key &key) const {
       return at(key);
   }

   iterator begin() {
       return iterator(findMin(root), this);
   }

   const_iterator cbegin() const {
       return const_iterator(findMin(root), this);
   }

   iterator end() {
       return iterator(nullptr, this);
   }

   const_iterator cend() const {
       return const_iterator(nullptr, this);
   }

   bool empty() const {
       return numElements == 0;
   }

   size_t size() const {
       return numElements;
   }

   void clear() {
       clearTree(root);
       root = nullptr;
       numElements = 0;
   }

   pair<iterator, bool> insert(const value_type &value) {
       pair<iterator, bool> result;
       root = insertNode(root, nullptr, value, result);
       return result;
   }

   void erase(iterator pos) {
       if (pos.container != this || pos == end()) {
           throw invalid_iterator();
       }
       bool found;
       root = eraseNode(root, pos.node->data->first, found);
       if (!found) {
           throw invalid_iterator();
       }
   }

   size_t count(const Key &key) const {
       return findNode(root, key) ? 1 : 0;
   }

   iterator find(const Key &key) {
       Node *node = findNode(root, key);
       return iterator(node, this);
   }

   const_iterator find(const Key &key) const {
       Node *node = findNode(root, key);
       return const_iterator(node, this);
   }
};

}

#endif