#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <initializer_list>
#include <vector>

template <typename Key, size_t N = 7/* implementation-defined */>
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = key_type&;
  using const_reference = const key_type&;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = Iterator;
  using const_iterator = Iterator;
  using key_compare = std::less<key_type>;   // B+-Tree
  using key_equal = std::equal_to<key_type>; // Hashing
  using hasher = std::hash<key_type>;        // Hashing

private:
  struct Node {
      Key data;
      Node *next = nullptr;
      Node *head = nullptr;
      ~Node() {
          delete head;
        delete next;
      };
  };
  Node *table = nullptr;
  size_type maxSize{N};
  size_type sz{0};

  void insert_unchecked(const_reference k) {
      
      Node *help = new Node;
      auto position = hash_idx(k);

      help->next = table[position].head;
      help->data = k;
      
      
      table[position].head = help;
      ++sz;

      if (maxSize * 10 < sz) rehash();
  }

  size_type hash_idx(const key_type& k) const { return hasher{}(k) % maxSize; }

  void rehash() {
       std::vector<Key> dataBuff;
       dataBuff.reserve(sz);

       for (const auto &k : *this)
           dataBuff.push_back(k);

       delete[] table;
       maxSize *= 2;
        sz = 0;

       table = new Node[maxSize];
       for (const auto &fromBuff : dataBuff)
           insert_unchecked(fromBuff);
       //std::cout << "table rehashed!" << '\n';
  }

public:
    
////////////////_CONSTRUCTORS_//////////////////////////////////////
  ADS_set() : table(new Node[N]) {
      //std::cout << "DEFAULT_CONSTRUCTOR" << '\n';
  }

  ADS_set(std::initializer_list<key_type> ilist) : ADS_set() {
      insert(ilist);
  }

  template<typename InputIt> ADS_set(InputIt first, InputIt last) : table(new Node[N]){
      insert(first, last);
      //std::cout << "RANGE_CONSTRUCTOR" << '\n';
  }

  ADS_set(const ADS_set &other) : table(new Node[N]) {
      for (const auto &key : other)
          insert_unchecked(key);
      //std::cout << "COPY_CONSTRUCTOR" << '\n';
  }

  ~ADS_set() { delete[] table; };

////////////////_METHODS_//////////////////////////////////////
  size_type size() const { return sz; }

  bool empty() const {
    if (sz == 0)
        return true;

    return false;
  }

  size_type count(const key_type &key) const {
     const auto row = hash_idx(key);                                             // hash the key to find apropriate row
     for (Node *node = table[row].head; node != nullptr; node = node->next)
         if (key_equal{}(node->data, key)) {
            return 1;
         }
     return 0;                                                                   // return 0 if key not found
  }

  iterator find(const key_type& key) const {
      const auto row = hash_idx(key);
         for (Node* n = table[row].head; n != nullptr; n = n->next)
             if (key_equal{}(n->data, key)) {
                 return const_iterator{ this, n, row, maxSize };
             }
      return end();
  }

  void swap(ADS_set& other){
      std::swap(maxSize, other.maxSize);
      std::swap(table, other.table);
      std::swap(sz, other.sz);
  }

  void insert(std::initializer_list<key_type> ilist) {
      for (auto const &element : ilist)                                         // iterate through the list
          if (!count(element))
              insert_unchecked(element);                                        // if element(key) already exists in table then skip
  }

  std::pair<iterator,bool> insert(const_reference key) {
      if (!count(key)) {
          insert_unchecked(key);
          return std::make_pair(find(key), true);
      }
      return std::make_pair(find(key), false);
  }

  template<typename InputIt> void insert(InputIt first, InputIt last) {
      for (auto it = first; it != last; ++it) insert(*it);
  }

  void clear() {
      delete[] table;
      sz = 0;
      maxSize = N;
      table = new Node[maxSize];
  }
    
  size_type erase (const key_type& key) {
      if (count(key)) {
          auto idx = hash_idx(key);
          
          Node *current = table[idx].head;
          Node *previous = nullptr;
          
          for (; current != nullptr; current = current->next) {
              if (key_equal{}(current->data, key)) {
                  if (current == table[idx].head) {
                      table[idx].head = current->next;
                      current->next = nullptr;
                      delete current;
                      
                      --sz;
                      return 1;
                  }
                  if (previous){
                      previous->next = current->next;
                      current->next = nullptr;
                      delete current;
                      
                      previous = nullptr;
                      delete previous;
                      
                      --sz;
                      return 1;
                  }
              }
              previous = current;
          }
      }
      return 0;
  }

  const_iterator begin() const {
      for (size_t i = 0; i < maxSize; i++)
          if (table[i].head) {
              return const_iterator(this, table[i].head, i, maxSize);
          }
      return end();
  }
    
  const_iterator end() const { return const_iterator(nullptr); }

  void dump(std::ostream& o = std::cerr) const {
      for (size_type i{0}; i < maxSize; ++i) {
          if (table[i].head == nullptr) {
              o << "[" << i << "]" << ": nullptr" << '\n';
              continue;
          };
          o << "[" << i << "]" << ": ";
          for (Node* a = table[i].head; a != nullptr; a = a->next) {
              o << a->data;
              if (a->next != nullptr)
              o << " -> ";
          }
          o << '\n';;
      }
  }
    
   ADS_set& operator= (const ADS_set& other) {
       clear();
        
       for (const auto &key : other)
           insert(key);
        
       return *this;
   }
    
   ADS_set& operator= (std::initializer_list<key_type> ilist) {
       clear();
       insert(ilist);
        
       return *this;
   }

  friend bool operator==(const ADS_set& lhs, const ADS_set& rhs) {
      if (lhs.sz != rhs.sz)
          return false;

      for (const auto &key : lhs)
          if (!rhs.count(key))
              return false;

      return true;
  }
    
  friend bool operator!=(const ADS_set& lhs, const ADS_set& rhs) {
      return !(lhs == rhs);
  }
};

///////////////////_ITERATOR_/////////////////////////////////////
template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
private:
  const ADS_set<Key, N> *ptr;
  Node *to;
  size_type itPos;
  size_type tblSz;

public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type&;
  using pointer = const value_type*;
  using iterator_category = std::forward_iterator_tag;

  explicit Iterator(const ADS_set *ads = nullptr, Node *tbl = nullptr,
                    size_type idx = 0, size_type sz = 0)
      : ptr(ads), to(tbl), itPos(idx), tblSz(sz){
    //std::cout << "ITERATOR_CONSTRUCTOR" << '\n';
  }

  reference operator*() const {
      return to->data;
  }

  pointer operator->() const {
      return &to->data;
  }

  Iterator& operator++() {
      while (itPos < tblSz) {
          if (to->next) { to = to->next; return *this; }
          else ++itPos;

          if (itPos == tblSz) { to = nullptr; return *this;} //!

          auto transit = ptr->table[itPos].head;
          if (transit) { to = transit; return *this; }
      }
      return *this;
  }

  Iterator operator++(int) {
      Iterator tmp(*this);
      operator++();

      return tmp;
  }

  friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
      return lhs.to == rhs.to;
  }
  friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
      return !(lhs.to == rhs.to);
  }
};

template <typename Key, size_t N> void swap(ADS_set<Key,N>& lhs, ADS_set<Key,N>& rhs) { lhs.swap(rhs); }

#endif // ADS_SET_H
