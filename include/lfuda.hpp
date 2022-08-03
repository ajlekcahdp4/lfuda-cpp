#pragma once

#include <algorithm>
#include <cassert>
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
        using local_list_t    = typename std::list<local_node_t>;
        using local_list_it_t = typename local_list_t::iterator;
        using freq_node_it_t  = typename std::map<freq_t, local_list_t>::iterator;

        T data_;
        KeyT key_;
        freq_t freq_;
        freq_node_it_t freq_list_;

      public:
        local_node_t (T data, freq_t freq, KeyT key) : key_ (key), freq_ (freq), data_ (data) {}
        local_node_t (T data, freq_t freq, KeyT key, freq_node_it_t pos)
            : key_ (key), freq_ (freq), data_ (data), freq_list_ (pos)
        {
        }

        T data () const { return data_; }
        KeyT key () const { return key_; }
        freq_t freq () const { return freq_; }
        freq_node_it_t freq_node () { return freq_list_; }

        freq_t set_freq (freq_t freq) { return freq_ = freq; }
        freq_t advance_freq () { return freq_++; }
        void set_freq_node (freq_node_it_t freq_node) { freq_list_ = freq_node; }
        freq_node_it_t &freq_list () { return freq_list_; }
    };
    using local_list_t    = typename std::list<local_node_t>;
    using local_list_it_t = typename local_list_t::iterator;
    using freq_node_it_t  = typename std::map<freq_t, local_list_t>::iterator;

    std::unordered_map<KeyT, local_list_it_t> table_;
    std::map<freq_t, local_list_t> rb_tree_;

    size_t capacity_ = 0;
    size_t size_     = 0;
    size_t age_      = 0;

  public:
    explicit lfuda_t (size_t cap) : capacity_ (cap), size_ (0) {}

    size_t size () const { return size_; }
    size_t age () const { return age_; }
    size_t capacity () const { return capacity_; }

    bool full () const { return size_ == capacity_; }

    template <typename F> bool lookup_update (KeyT key, F slow_get_page)
    {
        auto hit = table_.find (key);
        if ( hit == table_.end () )
        {
            if ( full () )
            {
                erase ();
            }
            insert (key, slow_get_page);

            return false;
        }
        auto found = hit->second;
        promote (found);

        return true;
    }

  private:
    void erase ()
    {
        // find to erase
        auto last_local_list_it = rb_tree_.begin ();
        auto &last_local_list   = last_local_list_it->second;
        auto freq               = last_local_list_it->first;
        auto to_erase_key       = last_local_list.back ().key ();
        assert (!last_local_list.empty ());

        // erase
        table_.erase (to_erase_key);
        last_local_list.pop_back ();
        age_ = freq;
        size_--;

        // remove if empty
        if ( last_local_list.empty () && freq != age_ + 1 )
            rb_tree_.erase (freq);   // log(N)
    }

    template <typename F> void insert (KeyT key, F slow_get_page)
    {
        // insert in rb tree or find local_list with freq == age_ + 1
        auto init_freq       = age_ + 1;
        auto ins_list_it     = rb_tree_.emplace (init_freq, local_list_t ()).first;   // log(N)
        auto &ins_local_list = ins_list_it->second;
        ins_local_list.emplace_front (local_node_t (slow_get_page (key), 1, key, ins_list_it));

        // insert request into table_ and rb_tree
        table_[key] = ins_local_list.begin ();
        size_++;
    }

    void promote (local_list_it_t &found)
    {
        found->advance_freq ();
        auto parent_list_it = found->freq_list ();
        auto &parent_list   = parent_list_it->second;

        // find new freq list for promoted local node
        auto new_freq         = age_ + found->freq ();
        auto next_list_it     = rb_tree_.emplace (new_freq, local_list_t ()).first;   // log(N)
        auto &next_local_list = next_list_it->second;

        // actually promote
        next_local_list.splice (next_local_list.begin (), parent_list, found);
        found->set_freq_node (next_list_it);

        // remove if empty
        if ( parent_list.empty () )
            rb_tree_.erase (parent_list_it->first);
    }
};
}   // namespace cache