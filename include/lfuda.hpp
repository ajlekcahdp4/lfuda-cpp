/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <alex.rom23@mail.ru> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Alex Romanov
 * ----------------------------------------------------------------------------
 */

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
      public:
        using local_list_t    = typename std::list<local_node_t>;
        using local_list_it_t = typename local_list_t::iterator;
        using freq_node_it_t  = typename std::map<freq_t, local_list_t>::iterator;

        T data_;
        KeyT key_;
        freq_t freq_;
        freq_node_it_t freq_list_;

        local_node_t (T data, freq_t freq, KeyT key) : key_ (key), freq_ (freq), data_ (data) {}
        local_node_t (T data, freq_t freq, KeyT key, freq_node_it_t pos)
            : key_ (key), freq_ (freq), data_ (data), freq_list_ (pos)
        {
        }
    };

    using local_list_t    = typename std::list<local_node_t>;
    using local_list_it_t = typename local_list_t::iterator;
    using freq_node_it_t  = typename std::map<freq_t, local_list_t>::iterator;

    std::unordered_map<KeyT, local_list_it_t> table_;
    std::map<freq_t, local_list_t> rb_tree_;

    size_t capacity_ = 0;
    size_t age_      = 0;

  public:
    lfuda_t (size_t cap) : capacity_ (cap) {}

    size_t size () const { return table_.size (); }
    size_t age () const { return age_; }
    size_t capacity () const { return capacity_; }

    bool full () const { return table_.size () == capacity_; }

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
        auto to_erase_key       = last_local_list.back ().key_;
        assert (!last_local_list.empty ());

        // erase
        table_.erase (to_erase_key);
        last_local_list.pop_back ();
        age_ = freq;

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
    }

    void promote (local_list_it_t &found)
    {
        found->freq_++;
        auto parent_list_it = found->freq_list_;
        auto &parent_list   = parent_list_it->second;

        // find new freq list for promoted local node
        auto new_freq         = age_ + found->freq_;
        auto next_list_it     = rb_tree_.emplace (new_freq, local_list_t ()).first;   // log(N)
        auto &next_local_list = next_list_it->second;

        // actually promote
        next_local_list.splice (next_local_list.begin (), parent_list, found);
        found->freq_list_ = next_list_it;

        // remove if empty
        if ( parent_list.empty () )
            rb_tree_.erase (parent_list_it->first);
    }
};
}   // namespace cache