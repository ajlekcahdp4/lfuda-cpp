#pragma once

#include <algorithm>
#include <iterator>
#include <list>
#include <map>
#include <unordered_map>

namespace cache
{

template <typename T> struct freq_node_t;

//===============================local_node and local_list===================================
template <typename T> struct local_node_t
{
    size_t freq;
    int key;
    freq_node_t<T> *freq_node;
    T data;

    local_node_t (size_t, int, freq_node_t<T> *);
    local_node_t () : freq (1) {}
};

// initializer
template <typename T>
local_node_t<T>::local_node_t (size_t fr, int key, freq_node_t<T> *freq_node_ar)
    : freq {fr}, key (key), freq_node (freq_node_ar)
{
}

template <typename T> int operator== (local_node_t<T> first, const local_node_t<T> second)
{
    return first.freq == second.freq;
}

template <typename T> using local_list_t = std::list<struct local_node_t<T>>;

template <typename T> using local_list_it_t = typename std::list<struct local_node_t<T>>::iterator;

//================================freq_node and freq_list======================================
template <typename T> struct freq_node_t
{
    size_t freq;

    local_list_t<T> local_list;

    freq_node_t (size_t);
    freq_node_t () : freq (1) {}
};

// initializer
template <typename T> freq_node_t<T>::freq_node_t (size_t fr) : freq {fr} {}

template <typename T> int operator== (freq_node_t<T> first, const freq_node_t<T> second)
{
    return first.freq == second.freq;
}

template <typename T> using freq_list_t = typename std::list<freq_node_t<T> *>;

template <typename T> using freq_list_it_t = typename std::list<freq_node_t<T> *>::iterator;

//==========================================LFU-DA=============================================

template <typename T, typename KeyT = int> class lfuda_t   // TODO: menage next freq
{
  private:
    freq_list_t<T> clist_;
    std::unordered_map<KeyT, local_list_it_t<T>> table_;
    std::map<T, KeyT> rbtree_;

    size_t capacity_;
    size_t size_;
    size_t age_;

  public:
    lfuda_t (size_t capacity) : capacity_ (capacity) {}

    size_t size () const { return size_; }
    size_t capacity () const { return capacity_; }
    size_t age () const { return age_; }

    bool full () const { return size_ == capacity_; }

    template <typename F> bool lookup_update (KeyT key, F slow_get_page)
    {
        auto hit = table_.find (key);

        if ( hit == table_.end () )   // 1. cache miss
        {
            if ( full () )
            {
                freq_node_t<T> &last_freq        = *clist_.front ();
                local_list_t<T> &last_local_list = last_freq.local_list;
                local_node_t<T> &to_erase        = last_freq.local_list.back ();
                table_.erase (table_.find (to_erase.key));
                last_local_list.pop_back ();

                if ( last_local_list.empty () )
                    clist_.erase (clist_.begin ());
                size_--;
            }

            freq_node_t<T> *first_freq = new freq_node_t<T> (age_);
            if ( clist_.empty () || (*clist_.front ()).freq != 1 )
                clist_.push_front (first_freq);
            else
            {
                delete first_freq;
                first_freq = clist_.front ();
            }

            local_node_t<T> *new_local_node = new local_node_t<T> (1, key, first_freq);
            new_local_node->data            = slow_get_page (key);
            new_local_node->freq_node->local_list.push_front (*new_local_node);
            table_[key] = first_freq->local_list.begin ();
            size_++;

            return false;
        }
        // 2. cache hit

        local_list_it_t<T> found       = hit->second;
        freq_node_t<T> &parent_node    = *(*found).freq_node;
        freq_list_it_t<T> it           = std::find (clist_.begin (), std::prev (clist_.end ()), &parent_node);
        freq_list_it_t<T> next_freq_it = std::next (it);

        size_t cur_freq          = parent_node.freq + 1;
        freq_node_t<T> *new_freq = new freq_node_t<T> (cur_freq);

        if ( next_freq_it == clist_.end () || (*next_freq_it)->freq != cur_freq )
            clist_.insert (std::next (it), new_freq);
        else
        {
            delete new_freq;
            new_freq = *(std::next (it));
        }

        local_list_t<T> &parent_local_list = parent_node.local_list;
        parent_local_list.splice (new_freq->local_list.begin (), parent_local_list, found, std::next (found));
        (*found).freq++;
        (*found).freq_node = new_freq;

        if ( parent_node.local_list.empty () )
            clist_.remove (&parent_node);

        return true;
    }
};

}   // namespace cache