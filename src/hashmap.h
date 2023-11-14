#pragma once

#include <limits>
#include <initializer_list>
#include <vector>
#include <stdexcept>
#include <new>
#include <cassert>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    HashMap(Hash hash_function = Hash());

    HashMap(std::size_t init_slots_size, Hash hash_function = Hash());

    template<class InputIt>
    HashMap(InputIt first, InputIt last, Hash hash_function = Hash());

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> init, Hash hash_function = Hash());

    std::size_t size() const;

    bool empty() const;

    Hash hash_function() const;

    void insert(std::pair<KeyType, ValueType> value);

    void erase(KeyType key);

    class iterator;

    class const_iterator;

    iterator begin();

    iterator end();

    const_iterator begin() const;

    const_iterator end() const;

    iterator find(KeyType key);

    const_iterator find(KeyType) const;

    ValueType &operator[](KeyType key);

    const ValueType &at(KeyType key) const;

    void clear();

private:
    using HashMapClass = HashMap<KeyType, ValueType, Hash>;

    Hash hash_function_;

    std::size_t size_;

    static const std::size_t NULL_INDEX = std::numeric_limits<std::size_t>::max();
    static const std::size_t DEFAULT_INIT_SLOTS_SIZE = 1024;
    static constexpr const double MIN_LOAD_FACTOR = 0.25;
    static constexpr const double MAX_LOAD_FACTOR = 0.8;

    struct Slot;

    std::vector<Slot> slots_;
    std::size_t largest_empty_;

    void init_empty_(size_t slots_size);

    std::size_t get_key_slot(const KeyType &key) const;

    void rehash_(size_t new_size_);
};


template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(Hash hash_function) : hash_function_(hash_function) {
    init_empty_(DEFAULT_INIT_SLOTS_SIZE);
}


template<class KeyType, class ValueType, class Hash>
template<class InputIt>
HashMap<KeyType, ValueType, Hash>::HashMap(InputIt first, InputIt last, Hash hash_function) : hash_function_(
        hash_function) {
    init_empty_(DEFAULT_INIT_SLOTS_SIZE);
    for (auto it = first; it != last; ++it) {
        insert(*it);
    }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(std::initializer_list<std::pair<KeyType, ValueType>> init,
                                           Hash hash_function) : HashMap(init.begin(), init.end(), hash_function) {}

template<class KeyType, class ValueType, class Hash>
std::size_t HashMap<KeyType, ValueType, Hash>::size() const {
    return size_;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::empty() const {
    return size_ == 0;
}

template<class KeyType, class ValueType, class Hash>
Hash HashMap<KeyType, ValueType, Hash>::hash_function() const {
    return hash_function_;
}

template<class KeyType, class ValueType, class Hash>
struct HashMap<KeyType, ValueType, Hash>::Slot {
    std::pair<const KeyType, ValueType> value;
    bool empty;
    std::size_t link;

    void init_empty() {
        empty = true;
        link = NULL_INDEX;
    }

    Slot() {
        init_empty();
    }

    void set_value(std::pair<KeyType, ValueType> new_value) {
        value.first.~KeyType();
        new(const_cast<KeyType *>(&value.first)) KeyType(new_value.first);
        value.second = new_value.second;
        empty = false;
    }

    Slot &operator=(const Slot &other) {
        if (&other == this) {
            return (*this);
        }

        value.first.~KeyType();
        new(const_cast<KeyType *>(&value.first)) KeyType(other.value.first);
        value.second = other.value.second;

        empty = other.empty;
        link = other.link;

        return (*this);
    }
};


template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::insert(std::pair<KeyType, ValueType> value) {
    std::size_t i = get_key_slot(value.first);

    if (slots_[i].empty) {
        slots_[i].set_value(value);
        ++size_;
    } else {
        while (!(slots_[i].value.first == value.first) && slots_[i].link != NULL_INDEX) {
            i = slots_[i].link;
        }

        if (!(slots_[i].value.first == value.first)) {
            while (!slots_[largest_empty_].empty) {
                assert(largest_empty_ != 0);
                --largest_empty_;
            }
            slots_[i].link = largest_empty_;
            slots_[largest_empty_].set_value(value);
            ++size_;
        }
    }

    if (size_ > MAX_LOAD_FACTOR * slots_.size()) {
        rehash_(2 * slots_.size());
    }
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::erase(KeyType key) {
    std::size_t i = get_key_slot(key);

    if (slots_[i].empty) {
        return;
    }

    std::size_t pi = NULL_INDEX;

    while (i != NULL_INDEX && slots_[i].value.first != key) {
        pi = i;
        i = slots_[i].link;
    }

    if (i != NULL_INDEX) {
        --size_;

        if (pi != NULL_INDEX) {
            slots_[pi].link = NULL_INDEX;
        }
        std::size_t hole = i;
        i = slots_[i].link;
        slots_[hole].link = NULL_INDEX;

        while (i != NULL_INDEX) {
            std::size_t j = get_key_slot(slots_[i].value.first);
            if (j == hole) {
                slots_[hole].set_value(slots_[i].value);
                hole = i;
            } else {
                while (slots_[j].link != NULL_INDEX) {
                    j = slots_[j].link;
                }
                slots_[j].link = i;
            }
            std::size_t k = slots_[i].link;
            slots_[i].link = NULL_INDEX;
            i = k;
        }

        slots_[hole].init_empty();
    }

    if (size_ < slots_.size() * MIN_LOAD_FACTOR) {
        rehash_((slots_.size() + 1) / 2);
    }
}

template<class KeyType, class ValueType, class Hash>
std::size_t HashMap<KeyType, ValueType, Hash>::get_key_slot(const KeyType &key) const {
    return hash_function_(key) % slots_.size();
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::begin() {
    return ++iterator(NULL_INDEX, &slots_);
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::end() {
    return iterator(slots_.size(), &slots_);
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::begin() const {
    return ++const_iterator(NULL_INDEX, (&slots_));
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::end() const {
    return const_iterator(slots_.size(), &slots_);
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::find(KeyType key) {
    std::size_t i = get_key_slot(key);

    if (slots_[i].empty) {
        return end();
    }

    while (slots_[i].link != NULL_INDEX && !(slots_[i].value.first == key)) {
        i = slots_[i].link;
    }

    if (slots_[i].value.first == key) {
        return iterator(i, &slots_);
    }
    return end();
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::find(KeyType key) const {
    std::size_t i = get_key_slot(key);

    if (slots_[i].empty) {
        return end();
    }

    while (slots_[i].link != NULL_INDEX && slots_[i].value.first != key) {
        i = slots_[i].link;
    }

    if (slots_[i].value.first == key) {
        return const_iterator(i, &slots_);
    }
    return end();
}

template<class KeyType, class ValueType, class Hash>
ValueType &HashMap<KeyType, ValueType, Hash>::operator[](KeyType key) {
    iterator it = find(key);
    if (it != end()) {
        return it->second;
    } else {
        insert(std::pair<KeyType, ValueType>(key, ValueType()));
        return find(key)->second;
    }
}

template<class KeyType, class ValueType, class Hash>
const ValueType &HashMap<KeyType, ValueType, Hash>::at(KeyType key) const {
    const_iterator it = find(key);
    if (it != end()) {
        return it->second;
    }
    throw std::out_of_range("");
}


template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::clear() {
    (*this) = HashMap<KeyType, ValueType, Hash>();
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::init_empty_(size_t slots_size) {
    slots_.assign(slots_size, Slot());
    largest_empty_ = slots_.size() - 1;
    size_ = 0;
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(std::size_t init_slots_size, Hash hash_function) {
    init_empty_(init_slots_size);
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::rehash_(size_t new_size_) {
    std::vector<std::pair<const KeyType, ValueType>> elements;
    for (const auto &x: *this) {
        elements.push_back(x);
    }
    init_empty_(new_size_);
    for (const auto &x: elements) {
        insert(x);
    }
}

template<class KeyType, class ValueType, class Hash>
class HashMap<KeyType, ValueType, Hash>::iterator {
public:
    iterator() {
        index_ = 0;
        slots_ = nullptr;
    }

    iterator(std::size_t index, std::vector<Slot> *slots) : index_(index), slots_(slots) {}

    iterator &operator++() {
        if (index_ == slots_->size()) {
            return *this;
        }
        ++index_;
        while (index_ < slots_->size() && (*slots_)[index_].empty) {
            ++index_;
        }
        return *this;
    }

    iterator operator++(int) {
        iterator ret_it = *this;
        ++(*this);
        return ret_it;
    }

    bool operator==(const iterator &other) {
        return index_ == other.index_ && slots_ == other.slots_;
    }

    bool operator!=(const iterator &other) {
        return !(*this == other);
    }

    std::pair<const KeyType, ValueType> &operator*() {
        return (*slots_)[index_].value;
    }

    std::pair<const KeyType, ValueType> *operator->() {
        return (&(*slots_)[index_].value);
    }

private:
    std::size_t index_;
    std::vector<Slot> *slots_;
};


template<class KeyType, class ValueType, class Hash>
class HashMap<KeyType, ValueType, Hash>::const_iterator {
public:
    const_iterator() {
        index_ = 0;
        slots_ = nullptr;
    }

    const_iterator(std::size_t index, const std::vector<Slot> *slots) : index_(index), slots_(slots) {}

    const_iterator &operator++() {
        if (index_ == slots_->size()) {
            return *this;
        }
        ++index_;
        while (index_ < slots_->size() && (*slots_)[index_].empty) {
            ++index_;
        }
        return *this;
    }

    const_iterator operator++(int) {
        const_iterator ret_it = *this;
        ++(*this);
        return ret_it;
    }

    bool operator==(const const_iterator &other) {
        return index_ == other.index_ && slots_ == other.slots_;
    }

    bool operator!=(const const_iterator &other) {
        return !(*this == other);
    }

    const std::pair<const KeyType, ValueType> &operator*() {
        return (*slots_)[index_].value;
    }

    const std::pair<const KeyType, ValueType> *operator->() {
        return &(*slots_)[index_].value;
    }

private:
    std::size_t index_;
    const std::vector<Slot> *slots_;
};