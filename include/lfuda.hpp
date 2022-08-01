#pragma once

#include <algorithm>
#include <iterator>
#include <list>
#include <map>
#include <unordered_map>

namespace cache
{
template <typename T, typename KeyT = int> class lfuda_t
{
    using freq_t = typename std::size_t;

    //=====================================local_node_t=========================================
    class local_node_t
    {
        using freq_node_it_t = typename std::map<KeyT, local_node_t>::iterator;

        T data_;
        KeyT key_;
        freq_t freq_;
        freq_node_it_t freq_node_;

      public:
        local_node_t (KeyT key, freq_t freq, T data) : key_ (key), freq_ (freq), data_ (data) {}

        T data () const { return data_; }
        KeyT key () const { return key_; }
        freq_t freq () const { return freq_; }

        freq_t set_freq (freq_t freq) { return freq_ = freq; }
        void set_freq_node (freq_node_it_t freq_node) { freq_node_ = freq_node; }
        freq_node_it_t &freq_node () { return freq_node_; }
    };
    using local_list_t    = typename std::list<local_node_t>;
    using local_list_it_t = typename local_list_t::iterator;
    using freq_node_it_t  = typename std::map<KeyT, local_node_t>::iterator;

    std::unordered_map<KeyT, local_list_it_t> table_;
    std::map<freq_t, local_list_t> rb_tree_;

    size_t capacity_ = 0;
    size_t size_     = 0;

  public:
    explicit lfuda_t (size_t cap) : capacity_ (cap) {}

    size_t size () const { return size_; }
    size_t capacity () const { return capacity_; }

    bool full () const { return size_ == capacity_; }

    template <typename F> bool lookup_update (KeyT key, F slow_get_page) {}
};
}   // namespace cache