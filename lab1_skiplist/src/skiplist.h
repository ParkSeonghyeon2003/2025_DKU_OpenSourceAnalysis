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
    ~SkipList();

    void Insert(const Key& key); // Insertion function (to be implemented by students)
    bool Contains(const Key& key) const; // Lookup function (to be implemented by students)
    std::vector<Key> Scan(const Key& key, const int scan_num) const; // Range query function (to be implemented by students)
    bool Delete(const Key& key); // Delete function (to be implemented by students)

    void Print() const;

   private:
    int RandomLevel() const; // Generates a random level for new nodes (to be implemented by students)

    Node* head; // Head node (starting point of the SkipList)
    int max_level; // Maximum level in the SkipList
    float probability; // Probability factor for level increase
    mutable std::mt19937 rng; // 랜덤 엔진
    mutable std::uniform_real_distribution<float> dist; // [0.0, 1.0) 균등분포 생성기
};

// SkipList Node structure
template<typename Key>
struct SkipList<Key>::Node {
    Key key;
    std::vector<Node*> next; // Pointer array for multiple levels

    // Constructor for Nodea
    // 키와 레벨을 지정하여 초기화
    Node(Key key, int level) : key(key), next(level, nullptr) {}
};

// Generate a random level for new nodes
template<typename Key>
int SkipList<Key>::RandomLevel() const {
    int level = 1;
    // 주어진 확률로 최대 레벨 이하까지 레벨 증가
    while(dist(rng) < probability && level < max_level) {
        level++;
    }
    return level;
}

// Constructor for SkipList
template<typename Key>
SkipList<Key>::SkipList(int max_level, float probability)
    : max_level(max_level), probability(probability) {
    // 헤드 노드를 최대 레벨로 초기화
    head = new Node(Key{}, max_level);
}

// 소멸자
template<typename Key>
SkipList<Key>::~SkipList() {
    Node* node = head;
    while (node) {
        Node* next = node->next[0];
        delete node;
        node = next;
    }
}

// Insert function (inserts a key into SkipList)
template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    std::vector<Node*> update(max_level); // 삽입 위치 추적
    Node* current = head;

    // 삽입 위치 찾기 (상위 레벨부터 아래로 탐색)
    for (int level = max_level - 1; level >= 0; --level) {
        // 다음 노드가 존재하고 키 값이 삽입할 키 값보다 작은 경우 계속 다음 노드로 이동
        while (current->next[level] != nullptr && current->next[level]->key < key) {
            current = current->next[level];
        }
        update[level] = current; // 삽입할 노드의 위치를 기억
    }
    
    // 이미 존재하는 키는 삽입 X
    current = current->next[0];
    if (current != nullptr && current->key == key) {
        return;
    }

    // 새로운 노드의 레벨 생성
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
    std::vector<Node*> update(max_level); // 삭제 위치 추적
    Node* current = head;

    // 삭제 대상 찾기 (Insert와 매커니즘 동일)
    for (int level = max_level - 1; level >= 0; --level) {
        while (current->next[level] && current->next[level]->key < key) {
            current = current->next[level];
        }
        update[level] = current;
    }

    current = current->next[0];

    // 삭제할 키가 없으면 false 반환
    if (current == nullptr || current->key != key) {
        return false;
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

    // 대상 찾기 (Insert와 매커니즘 동일)
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
std::vector<Key> SkipList<Key>::Scan(const Key& key, const int scan_num) const {
    std::vector<Key> result;
    Node* current = head;

    // key 이상의 노드 찾기
    for (int level = max_level - 1; level >= 0; --level) {
        while (current->next[level] && current->next[level]->key < key) {
            current = current->next[level];
        }
    }

    current = current->next[0];

    // 다음 노드들을 차례로 scan_num개를 수집한다.
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
