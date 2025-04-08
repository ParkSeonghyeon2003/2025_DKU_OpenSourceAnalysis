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

typedef std::chrono::high_resolution_clock Clock;

// Key is an 8-byte integer
typedef uint64_t Key;

int compare_(const Key& a, const Key& b) {
    if (a < b) {
        return -1;
    } else if (a > b) {
        return +1;
    } else {
        return 0;
    }
}

template<typename Key>
class SkipList {
   private:
    struct Node;

   public:
    SkipList(int max_level = 16, float probability = 0.5);

    void Insert(const Key& key); // Insertion function (to be implemented by students)
    bool Contains(const Key& key) const; // Lookup function (to be implemented by students)
    std::vector<Key> Scan(const Key& key, const int scan_num); // Range query function (to be implemented by students)
    bool Delete(const Key& key); // Delete function (to be implemented by students)

    void Print() const;

   private:
    int RandomLevel(); // Generates a random level for new nodes (to be implemented by students)

    Node* head; // Head node (starting point of the SkipList)
    int max_level; // Maximum level in the SkipList
    float probability; // Probability factor for level increase
    mutable std::mt19937 rng; // 랜덤 엔진
    mutable std::uniform_real_distribution<float> dist;
};

// SkipList Node structure
template<typename Key>
struct SkipList<Key>::Node {
    Key key;
    std::vector<Node*> next; // Pointer array for multiple levels

    // Constructor for Node
    Node(Key key, int level);
};

// Generate a random level for new nodes
template<typename Key>
int SkipList<Key>::RandomLevel() {
    int level = 1;
    while(dist(rng) < probability && level < max_level) {
        level++;
    }
    return level;
}

// Constructor for SkipList
template<typename Key>
SkipList<Key>::SkipList(int max_level, float probability)
    : max_level(max_level), probability(probability) {
    head = new Node(Key{}, max_level);
}

// Insert function (inserts a key into SkipList)
template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    std::vector<Node*> update(max_level);
    Node* current = head;

    // 삽입 위치 찾기
    for (int level = max_level - 1; level >= 0; --level) {
        while (current->next[level] != nullptr && current->next[level]->key < key) {
            current = current->next[level];
        }
        update[level] = current;
    }

    current = current->next[0];
    if (current != nullptr && current->key == key) {
        return; // 이미 존재하는 키는 삽입 X
    }

    int node_level = RandomLevel();
    Node* new_node = new Node(key, node_level);

    // 각 레벨에 새 노드 연결
    for (int i = 0; i < node_level; ++i) {
        new_node->next[i] = update[i]->next[i];
        update[i]->next[i] = new_node;
    }
}

// Delete function (removes a key from SkipList)
template<typename Key>
bool SkipList<Key>::Delete(const Key& key) {
    std::vector<Node*> update(max_level);
    Node* current = head;

    // 삭제 대상 찾기
    for (int level = max_level - 1; level >= 0; --level) {
        while (current->next[level] && current->next[level]->key < key) {
            current = current->next[level];
        }
        update[level] = current;
    }

    current = current->next[0];
    if (current == nullptr || current->key != key) {
        return false; // 존재하지 않음
    }

    // 연결 끊기
    for (int i = 0; i < max_level; ++i) {
        if (update[i]->next[i] != current) break;
        update[i]->next[i] = current->next[i];
    }

    delete current;
    return true;
}

// Lookup function (checks if a key exists in SkipList)
template<typename Key>
bool SkipList<Key>::Contains(const Key& key) const {
    Node* current = head;
    for (int level = max_level - 1; level >= 0; --level) {
        while (current->next[level] && current->next[level]->key < key) {
            current = current->next[level];
        }
    }
    current = current->next[0];
    return current != nullptr && current->key == key;
}

// Range query function (retrieves scan_num keys starting from key)
template<typename Key>
std::vector<Key> SkipList<Key>::Scan(const Key& key, const int scan_num) {
    std::vector<Key> result;
    Node* current = head;

    // key 이상의 노드 찾기
    for (int level = max_level; level >= 0; --level) {
        while (current->next[level] && current->next[level]->key < key) {
            current = current->next[level];
        }
    }

    current = current->next[0];
    while (current && result.size() < static_cast<size_t>(scan_num)) {
        result.push_back(current->key);
        current = current->next[0];
    }

    return result;
}

template<typename Key>
void SkipList<Key>::Print() const {
  std::cout << "SkipList Structure:\n";
  for (int level = max_level - 1; level >= 0; --level) {
    Node* node = head->next[level];
    std::cout << "Level " << level << ": ";
    while (node != nullptr) {
      std::cout << node->key << " ";
      node = node->next[level];
    }
    std::cout << "\n";
  }
}
