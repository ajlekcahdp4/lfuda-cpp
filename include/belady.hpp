/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <alex.rom23@mail.ru> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.       Alex Romanov
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <deque>
#include <iterator>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace cache
{
template <typename T, typename iterator_t> class ideal_cache_t
{
    using occurence_map = typename std::unordered_map<T, std::deque<int>>;

    occurence_map occur_map_;
    std::map<int, T> cmap_;
    std::unordered_set<T> inserted_set_;
    size_t capacity_;
    size_t test_size_ = 0;
    size_t size_      = 0;

    void fill (iterator_t begin, iterator_t end)
    {
        if ( begin == end )
            throw std::invalid_argument ("Wrong argument(s) in get_best_hits_count ()");

        size_t step {};
        for ( ; begin != end; begin++, step++ )
        {
            auto &ins_que = occur_map_.emplace (*begin, std::deque<int> ()).first->second;
            ins_que.push_back (step);
        }
    }

  public:
    bool full () const { return size_ == capacity_; }

    ideal_cache_t (size_t m, size_t n, iterator_t begin, iterator_t end) : capacity_ (m), test_size_ (n)   // O(n)
    {
        fill (begin, end);
    }

    bool lookup_update (const T &to_insert)
    {
        auto found = inserted_set_.find (to_insert);
        if ( found == inserted_set_.end () )
        {
            if ( full () )
                erase ();
            insert (to_insert);
            return false;
        }
        promote (to_insert);
        return true;
    }

  private:
    void erase ()
    {
        auto to_erase_it = std::prev (cmap_.end ());
        inserted_set_.erase (to_erase_it->second);
        cmap_.erase (to_erase_it);   // log (N)
        size_--;

        assert (cmap_.size () == size_);
    }

    void insert (const T &to_insert)
    {
        auto new_soon = promote_que (to_insert);
        if ( new_soon )   // if promote_que returned 0, than we don't need to insert
        {
            inserted_set_.insert (to_insert);
            cmap_.insert ({new_soon, to_insert});
            size_++;
        }
        assert (cmap_.size () == size_);
    }

    void promote (const T &to_promote)
    {
        auto found = occur_map_.find (to_promote);
        assert (found != occur_map_.end ());
        auto &que = found->second;

        cmap_.erase (que.front ());

        que.pop_front ();
        if ( que.empty () )
        {
            occur_map_.erase (to_promote);
            size_--;
        }
        else
            cmap_.insert ({que.front (), to_promote});
        assert (cmap_.size () == size_);
    }

    size_t promote_que (const T &to_promote)
    {
        auto found = occur_map_.find (to_promote);
        assert (found != occur_map_.end ());
        auto &que = found->second;

        que.pop_front ();
        if ( que.empty () )
        {
            occur_map_.erase (to_promote);
            return 0;
        }
        return que.front ();
    }
};

template <typename T, typename iterator_t>
size_t get_best_hits_count (size_t size, size_t len, iterator_t begin, iterator_t end)
{
    if ( !size || begin == end )
        throw std::invalid_argument ("Wrong argument(s) in get_best_hits_count ()");

    ideal_cache_t<T, iterator_t> cache {size, len, begin, end};

    size_t hits {};
    for ( ; begin != end; begin++ )
    {
        if ( cache.lookup_update (*begin) )
            hits++;
    }
    return hits;
}
}   // namespace cache