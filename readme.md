# Coalesced HashMap

This is a hashmap that uses coalesced hashing.

The implementation is based on the paper "Implementations for Coalesced Hashing" by Jeffrey Scott Vitter from Brown University.

The hashmap can dynamically change its capacity to guarantee fast execution of operations.

# Usage

The hashmap has the following template parameters:
- KeyType
- ValueType
- Hash

It has the following constructors:
- HashMap(Hash hash_function = Hash())
- HashMap(std::size_t init_slots_size, Hash hash_function = Hash());
- HashMap(InputIt first, InputIt last, Hash hash_function = Hash());
- HashMap(std::initializer_list<std::pair<KeyType, ValueType>> init, Hash hash_function = Hash());

It has the following methods:
- std::size_t size() const;
- bool empty() const;
- Hash hash_function() const;
- void insert(std::pair<KeyType, ValueType> value);
- void erase(KeyType key);
- iterator begin();
- iterator end();
- const_iterator begin() const;
- const_iterator end() const;
- iterator find(KeyType key);
- const_iterator find(KeyType) const;
- ValueType &operator[](KeyType key);
- const ValueType &at(KeyType key) const;
- void clear();
