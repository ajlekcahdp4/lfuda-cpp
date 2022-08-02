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
    size_t age_      = 1;

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
        auto last_local_list_it       = rb_tree_.begin ();
        local_list_t &last_local_list = last_local_list_it->second;
        auto freq                     = last_local_list_it->first;
        auto &to_erase                = last_local_list.back ();

        // erase
        table_.erase (to_erase.key ());
        last_local_list.pop_back ();
        age_ = freq;
        size_--;

        // remove if empty
        if ( last_local_list.empty () && freq != age_ )
            rb_tree_.erase (to_erase.freq ());   // log(N)
    }

    template <typename F> void insert (KeyT key, F slow_get_page)
    {
        // insert in rb tree or find local_list with freq == age_
        freq_node_it_t ins_list_it = rb_tree_.find (age_);   // log(N)
        if ( rb_tree_.empty () || ins_list_it == rb_tree_.end () )
            ins_list_it = rb_tree_.emplace (age_, local_list_t ()).first;   // log(N)
        local_list_t &ins_list = ins_list_it->second;
        ins_list.emplace_front (local_node_t (slow_get_page (key), age_, key, ins_list_it));
        // insert request into table_ and rb_tree
        table_[key] = ins_list.begin ();
        size_++;
    }

    void promote (local_list_it_t &found)
    {
        freq_node_it_t parent_list_it = found->freq_list ();
        freq_node_it_t next_list_it   = std::next (parent_list_it);
        auto &parent_list             = parent_list_it->second;
        auto &next_list               = next_list_it->second;

        // find new freq list for promoted local node
        auto new_freq              = age_ + found->freq () + 1;
        freq_node_it_t ins_list_it = next_list_it;
        if ( next_list_it == rb_tree_.end () || next_list_it->first != new_freq )
            ins_list_it = rb_tree_.emplace (new_freq, local_list_t ()).first;   // log(N)
        local_list_t &next_freq_list = ins_list_it->second;

        // actually promote
        next_freq_list.splice (next_freq_list.begin (), parent_list, found);

        // remove if empty
        if ( parent_list.empty () )
            rb_tree_.erase (parent_list_it->first);
    }
};
}   // namespace cache

// insert freq_list into local_node