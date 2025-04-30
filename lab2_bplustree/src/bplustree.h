#include <stdlib.h>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <bit>
#include <functional>
#include <mutex>
#include <vector>

#include <atomic>

// Define Clock and Key types
typedef std::chrono::high_resolution_clock Clock;
typedef uint64_t Key;

// Compare function for keys
int compare_(const Key& a, const Key& b) {
    if (a < b) {
        return -1;
    } else if (a > b) {
        return +1;
    } else {
        return 0;
    }
}

// B+ Tree class template definition
template<typename Key>
class Bplustree {
   private:
    // Forward declaration of node structures
    struct Node;
    struct InternalNode;
    struct LeafNode;

   public:
    // Constructor: Initializes a B+ Tree with the specified degree (maximum number of children per internal node)
    Bplustree(int degree = 4);

    // Insert function:
    // Inserts a key into the B+ Tree.
    // TODO: Implement insertion, handling leaf node insertion and splitting if necessary.
    void Insert(const Key& key);

    // Contains function:
    // Returns true if the key exists in the tree; otherwise, returns false.
    // TODO: Implement key lookup starting from the root and traversing to the appropriate leaf.
    bool Contains(const Key& key) const;

    // Scan function:
    // Performs a range query starting from the specified key and returns up to 'scan_num' keys.
    // TODO: Traverse leaf nodes using the next pointer and collect keys.
    std::vector<Key> Scan(const Key& key, const int scan_num);

    // Delete function:
    // Removes the specified key from the tree.
    // TODO: Implement deletion, handling key removal, merging, or rebalancing nodes if required.
    bool Delete(const Key& key);

    // Print function:
    // Traverses and prints the internal structure of the B+ Tree.
    // This function is helpful for debugging and verifying that the tree is constructed correctly.
    void Print() const;

   private:
    // Base Node structure. All nodes (internal and leaf) derive from this.
    struct Node {
        bool is_leaf; // Indicates whether the node is a leaf
        // Helper functions to cast a Node pointer to InternalNode or LeafNode pointers.
        InternalNode* as_internal() { return static_cast<InternalNode*>(this); }
        LeafNode* as_leaf() { return static_cast<LeafNode*>(this); }
        virtual ~Node() = default;
    };

    // Internal node structure for the B+ Tree.
    // Stores keys and child pointers.
    struct InternalNode : public Node {
        std::vector<Key> keys;         // Keys used to direct search to the correct child
        std::vector<Node*> children;   // Pointers to child nodes
        InternalNode() { this->is_leaf = false; }
    };

    // Leaf node structure for the B+ Tree.
    // Stores actual keys and a pointer to the next leaf for efficient range queries.
    struct LeafNode : public Node {
        std::vector<Key> keys; // Keys stored in the leaf node
        LeafNode* next;        // Pointer to the next leaf node for range scanning
        LeafNode() : next(nullptr) { this->is_leaf = true; }
    };

    // Helper function to insert a key into an internal node.
    // 'new_child' and 'new_key' are output parameters if the node splits.
    // TODO: Implement insertion into an internal node and handle splitting of nodes.
    void InsertInternal(Node* current, const Key& key, Node*& new_child, Key& new_key);

    // Helper function to delete a key from the tree recursively.
    // TODO: Implement deletion from internal nodes with proper merging or rebalancing.
    bool DeleteInternal(Node* current, const Key& key);

    // Helper function to find the leaf node where the key should reside.
    // TODO: Implement traversal from the root to the appropriate leaf node.
    LeafNode* FindLeaf(const Key& key) const;

    // Helper function to recursively print the tree structure.
    void PrintRecursive(const Node* node, int level) const;

    Node* root;   // Root node of the B+ Tree
    int degree;   // Maximum number of children per internal node
};

// Constructor implementation
// Initializes the tree by creating an empty leaf node as the root.
template<typename Key>
Bplustree<Key>::Bplustree(int degree) : degree(degree) {
    root = new LeafNode();
    // To be implemented by students
}

// Insert function: Inserts a key into the B+ Tree.
template<typename Key>
void Bplustree<Key>::Insert(const Key& key) {
    // TODO: Implement insertion logic here.
    LeafNode* leaf = FindLeadf(key);

    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    if (it != leaf->keys.end() && *it == key) return; // 중복 방지
    
    leaf->keys.insert(it, key);

    if (leaf->keys.size() < degree) return; // 분할 불필요

    // 분할 필요
    LeafNode* new_leaf = new LeafNode();
    int mid = leaf->keys.size() / 2;

    new_leaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
    leaf->keys.resize(mid);

    new_leaf->next = leaf->next;
    leaf->next = new_leaf;

    Key new_key = new_leaf->keys.front(); // 부모로 올릴 key

    if (leaf == root) {
        // 루트 분할
        InternalNode* new_root = new InternalNode();
        new_root->keys.push_back(new_key);
        new_root->children.push_back(leaf);
        new_root->children.push_back(new_leaf);
        root = new_root;
    } else {
        // 루트 외 삽입
        Node* new_child = new_leaf;
        InsertInternal(root, new_key, new_child, new_key);
    }
}


// Contains function: Checks if a key exists in the B+ Tree.
template<typename Key>
bool Bplustree<Key>::Contains(const Key& key) const {
    // TODO: Implement lookup logic here.
    LeafNode* leaf = FindLeaf(key);
    return std::binary_search(leaf->keys.begin(), leaf->keys.end(), key);
}


// Scan function: Performs a range query starting from a given key.
template<typename Key>
std::vector<Key> Bplustree<Key>::Scan(const Key& key, const int scan_num) {
    // TODO: Implement range query logic here.
    std::vector<Key> result;
    LeafNode* leaf = FindLeaf(key);
    while (leaf && result.size() < scan_num) {
        for (const Key& k : leaf->keys) {
            if (k >= key) result.push_back(k);
            if (result.size() == scan_num) break;
        }
        leaf = leaf->next;
    }
    return result;
}


// Delete function: Removes a key from the B+ Tree.
template<typename Key>
bool Bplustree<Key>::Delete(const Key& key) {
    // TODO: Implement deletion logic here.
    // To be implemented by students
    return false;
}


// InsertInternal function: Helper function to insert a key into an internal node.
template<typename Key>
void Bplustree<Key>::InsertInternal(Node* current, const Key& key, Node*& new_child, Key& new_key) {
    // TODO: Implement internal node insertion logic here.
    if (current->is_leaf) return; // 잘못된 호출 방지

    InternalNode* internal = current->as_internal();
    size_t i = 0;
    while (i < internal->keys.size() && key >= internal->keys[i]) ++i;

    Node* child = internal->children[i];

    if (child->is_leaf) {
        // 이미 처리한 경우
        internal->keys.insert(internal->keys.begin() + i, new_key);
        internal->children.insert(internal->children.begin() + i + 1, new_child);
    } else {
        Node* new_grandchild = nullptr;
        Key promoted_key;
        InsertInternal(chidl, key, new_grandchild, promoted_key);
        if (new_grandchild != nullptr) {
            internal->keys.insert(internal->keys.begin() + 1, promoted_key);
            internal->children.insert(internal->children.begin() + i + 1, new_grandchild);
        }
    }

    if (internal->keys.size() >= degree) {
        InternalNode* new_internal = nre InternalNode();
        int mid = internal->keys.size() / 2;

        new_key = internal->keys[mid];
        new_internal->keys.assign(internal->keys.begin() + mid + 1, internal->keys.end());
        new_internal->children.assign(internal->children.begin() + mid + 1, internal->children.end());

        internal->keys.resize(mid);
        internal->children.resize(mid + 1);

        new_child = new_internal;

        if (current == root) {
            InternalNode* new_root = new InternalNode();
            new_root->keys.push_back(new_key);
            new_root->children.push_back(internal);
            new_root->children.push_back(new_child);
            root = new_root;
            new_child = nullptr;
        }
    } else {
        new_child = nullptr;
    }
}


// DeleteInternal function: Helper function to delete a key from an internal node.
template<typename Key>
bool Bplustree<Key>::DeleteInternal(Node* current, const Key& key) {
    // TODO: Implement internal node deletion logic here.
    // To be implemented by students
    return false;
}


// FindLeaf function: Traverses the B+ Tree from the root to find the leaf node that should contain the given key.
template<typename Key>
typename Bplustree<Key>::LeafNode* Bplustree<Key>::FindLeaf(const Key& key) const {
    // TODO: Implement the traversal logic to locate the correct leaf node.
    Node* current = root;
    while (!current->is_leaf) {
        InternalNode* internal = current->as_internal();
        size_t i = 0;
        while (i < internal->keys.size() && key >= internal->keys[i]) {
            ++i;
        }
        current = internal->children[i];
    }
    return current->as_leaf();
}

// Print function: Public interface to print the B+ Tree structure.
template<typename Key>
void Bplustree<Key>::Print() const {
    PrintRecursive(root, 0);
}

// Helper function: Recursively prints the tree structure with indentation based on tree level.
template<typename Key>
void Bplustree<Key>::PrintRecursive(const Node* node, int level) const {
    if (node == nullptr) return;
    // Indent based on the level in the tree.
    for (int i = 0; i < level; ++i)
        std::cout << "  ";
    if (node->is_leaf) {
        // Print leaf node keys.
        const LeafNode* leaf = node->as_leaf();
        std::cout << "[Leaf] ";
        for (const Key& key : leaf->keys)
            std::cout << key << " ";
        std::cout << std::endl;
    } else {
        // Print internal node keys and recursively print children.
        const InternalNode* internal = node->as_internal();
        std::cout << "[Internal] ";
        for (const Key& key : internal->keys)
            std::cout << key << " ";
        std::cout << std::endl;
        for (const Node* child : internal->children)
            PrintRecursive(child, level + 1);
    }
}