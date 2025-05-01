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
        LeafNode* as_leaf() { return static_cast<LeafNode*>(this); }
        const LeafNode* as_leaf() const { return static_cast<const LeafNode*>(this); }

        InternalNode* as_internal() { return static_cast<InternalNode*>(this); }
        const InternalNode* as_internal() const { return static_cast<const InternalNode*>(this); }

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
    LeafNode* leaf = FindLeaf(key); // 키가 들어갈 리프 찾기
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key); // 정렬 유지하며 위치 찾기
    if (it != leaf->keys.end() && *it == key) return; // 중복이면 삽입하지 않음

    leaf->keys.insert(it, key); // 키 삽입

    if (leaf->keys.size() < degree) return; // 분할 필요 없음

    // 노드 분할
    LeafNode* new_leaf = new LeafNode();
    int mid = leaf->keys.size() / 2;

    new_leaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end()); // 오른쪽 절반 복사
    leaf->keys.resize(mid); // 왼쪽 절반 유지

    new_leaf->next = leaf->next; // next 포인터 조정
    leaf->next = new_leaf;

    Key new_key = new_leaf->keys.front(); // 부모에 올릴 키

    if (leaf == root) {
        // 루트였으면 새로운 루트 생성
        InternalNode* new_root = new InternalNode();
        new_root->keys.push_back(new_key);
        new_root->children.push_back(leaf);
        new_root->children.push_back(new_leaf);
        root = new_root;
    } else {
        // 루트가 아니면 내부 노드에 삽입 요청
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
    if (!Contains(key)) return false; // 없으면 삭제 실패

    LeafNode* leaf = FindLeaf(key);
    auto it = std::find(leaf->keys.begin(), leaf->keys.end(), key);
    if (it != leaf->keys.end()) leaf->keys.erase(it); // 삭제

    if (leaf == root) {
        if (leaf->keys.empty()) {
            delete root;
            root = new LeafNode();
        }
        return true;
    }

    int min_keys = std::ceil(degree / 2.0);
    if (leaf->keys.size() < min_keys) {
        DeleteInternal(root, key); // 부모에게 구조 조정 요청
    }

    return true;
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
        InsertInternal(child, key, new_grandchild, promoted_key);
        if (new_grandchild != nullptr) {
            internal->keys.insert(internal->keys.begin() + i, promoted_key);
            internal->children.insert(internal->children.begin() + i + 1, new_grandchild);
        }
    }

    if (internal->keys.size() >= degree) {
        InternalNode* new_internal = new InternalNode();
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
    if (current->is_leaf) return false; // 이 함수는 Internal 노드에서만 호출됨

    InternalNode* internal = current->as_internal();
    size_t i = 0;

    // 적절한 자식 인덱스를 찾음
    while (i < internal->keys.size() && key >= internal->keys[i]) ++i;
    Node* child = internal->children[i];

    // 리프 노드 처리
    if (child->is_leaf) {
        LeafNode* leaf = child->as_leaf();
        auto it = std::find(leaf->keys.begin(), leaf->keys.end(), key);
        if (it == leaf->keys.end()) return false; // 키 없음

        // 키 삭제
        leaf->keys.erase(it);

        // 최소 키 수 이상이면 OK
        int min_keys = std::ceil(degree / 2.0);
        if (leaf->keys.size() >= min_keys) return true;

        // 재분배 또는 병합
        bool merged = false;

        // 왼쪽 형제에서 빌리기
        if (i > 0) {
            LeafNode* left = internal->children[i - 1]->as_leaf();
            if (left->keys.size() > min_keys) {
                leaf->keys.insert(leaf->keys.begin(), left->keys.back());
                left->keys.pop_back();
                internal->keys[i - 1] = leaf->keys.front();
                return true;
            }
        }

        // 오른쪽 형제에서 빌리기
        if (i + 1 < internal->children.size()) {
            LeafNode* right = internal->children[i + 1]->as_leaf();
            if (right->keys.size() > min_keys) {
                leaf->keys.push_back(right->keys.front());
                right->keys.erase(right->keys.begin());
                internal->keys[i] = right->keys.front();
                return true;
            }
        }

        // 병합 시도
        if (i > 0) {
            // 왼쪽과 병합
            LeafNode* left = internal->children[i - 1]->as_leaf();
            left->keys.insert(left->keys.end(), leaf->keys.begin(), leaf->keys.end());
            left->next = leaf->next;
            delete leaf;
            internal->children.erase(internal->children.begin() + i);
            internal->keys.erase(internal->keys.begin() + i - 1);
            merged = true;
        } else if (i + 1 < internal->children.size()) {
            // 오른쪽과 병합
            LeafNode* right = internal->children[i + 1]->as_leaf();
            leaf->keys.insert(leaf->keys.end(), right->keys.begin(), right->keys.end());
            leaf->next = right->next;
            delete right;
            internal->children.erase(internal->children.begin() + i + 1);
            internal->keys.erase(internal->keys.begin() + i);
            merged = true;
        }

        // 루트가 비어있으면 루트 축소
        if (internal == root && internal->keys.empty()) {
            root = internal->children[0];
            delete internal;
        }

        return merged;
    }

    // 내부 노드일 경우 재귀 호출
    bool deleted = DeleteInternal(child, key);

    // 이후 병합이나 재분배 필요 여부 확인
    InternalNode* child_internal = child->as_internal();
    int min_children = std::ceil(degree / 2.0);
    if (child_internal->children.size() >= min_children) return deleted;

    // 재분배 또는 병합
    if (i > 0) {
        InternalNode* left = internal->children[i - 1]->as_internal();
        if (left->children.size() > min_children) {
            // 왼쪽 형제에서 빌려오기
            child_internal->children.insert(child_internal->children.begin(), left->children.back());
            left->children.pop_back();
            child_internal->keys.insert(child_internal->keys.begin(), internal->keys[i - 1]);
            internal->keys[i - 1] = left->keys.back();
            left->keys.pop_back();
            return true;
        }
    }

    if (i + 1 < internal->children.size()) {
        InternalNode* right = internal->children[i + 1]->as_internal();
        if (right->children.size() > min_children) {
            // 오른쪽 형제에서 빌려오기
            child_internal->children.push_back(right->children.front());
            right->children.erase(right->children.begin());
            child_internal->keys.push_back(internal->keys[i]);
            internal->keys[i] = right->keys.front();
            right->keys.erase(right->keys.begin());
            return true;
        }
    }

    // 병합
    if (i > 0) {
        InternalNode* left = internal->children[i - 1]->as_internal();
        left->keys.push_back(internal->keys[i - 1]);
        left->keys.insert(left->keys.end(), child_internal->keys.begin(), child_internal->keys.end());
        left->children.insert(left->children.end(), child_internal->children.begin(), child_internal->children.end());
        delete child_internal;
        internal->children.erase(internal->children.begin() + i);
        internal->keys.erase(internal->keys.begin() + i - 1);
    } else if (i + 1 < internal->children.size()) {
        InternalNode* right = internal->children[i + 1]->as_internal();
        child_internal->keys.push_back(internal->keys[i]);
        child_internal->keys.insert(child_internal->keys.end(), right->keys.begin(), right->keys.end());
        child_internal->children.insert(child_internal->children.end(), right->children.begin(), right->children.end());
        delete right;
        internal->children.erase(internal->children.begin() + i + 1);
        internal->keys.erase(internal->keys.begin() + i);
    }

    // 루트가 비어 있다면 축소
    if (internal == root && internal->keys.empty()) {
        root = internal->children[0];
        delete internal;
    }

    return true;
}

// FindLeaf function: Traverses the B+ Tree from the root to find the leaf node that should contain the given key.
// FindLeaf 함수: 키가 삽입/검색/삭제될 위치를 찾기 위해 루트부터 리프까지 내려가는 함수
template<typename Key>
typename Bplustree<Key>::LeafNode* Bplustree<Key>::FindLeaf(const Key& key) const {
    // TODO: Implement the traversal logic to locate the correct leaf node.
    Node* current = root;
    // leaf까지 내려가는 루프
    while (!current->is_leaf) {
        InternalNode* internal = current->as_internal();
        size_t i = 0;
        // 키를 왼쪽에서 부터 찾으려는 키 전까지 (혹은 끝까지) i가 인덱스로 가르킴
        while (i < internal->keys.size() && key >= internal->keys[i]) {
            ++i;
        }
        current = internal->children[i]; // 자식으로 내려감
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