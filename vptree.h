#ifndef VPTREE_H
#define VPTREE_H

#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <queue>
#include <limits>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "dataset.h"

namespace clustering {

template < typename T, double ( *distance )( const T&, const T& ) >
class VPTREE {
public:
    typedef boost::shared_ptr< VPTREE > Ptr;
    typedef std::pair< size_t, float > TNeighbor;
    typedef std::vector< TNeighbor > TNeighborsList;

    static const uint32_t FIRTS_NODE_IDX = 1;

    VPTREE()
        : m_root_idx( FIRTS_NODE_IDX )
        , m_next_idx( FIRTS_NODE_IDX )
    {
    }

    ~VPTREE()
    {
    }

    void create( const Dataset::Ptr dataset )
    {
        m_next_idx = FIRTS_NODE_IDX;
        m_root_idx = FIRTS_NODE_IDX;
        m_dataset = dataset;

        const Dataset::DataContainer& d = m_dataset->data();

        m_nodelist.resize( d.size() + 1 );
        m_items_idx.resize( d.size() );

        for ( size_t i = 0; i < d.size(); ++i ) {
            m_items_idx[i] = i;
        }

        m_root_idx = buildFromPoints( 0, d.size(), d );
    }

    void search( const T& target, double t, TNeighborsList& nlist ) const
    {
        nlist.clear();

        const Dataset::DataContainer& d = m_dataset->data();
        std::priority_queue< HeapItem > heap;

        search( m_root_idx, target, nlist, t, d );
    }

private:
    struct Node {
        uint32_t index;
        double threshold;
        uint32_t left;
        uint32_t right;

        Node()
            : index( 0 )
            , threshold( 0. )
            , left( 0 )
            , right( 0 )
        {
        }
    };

    uint32_t m_root_idx;
    uint32_t m_next_idx;
    typedef std::vector< Node > TNodeList;

    TNodeList m_nodelist;

    Dataset::Ptr m_dataset;
    std::vector< size_t > m_items_idx;

    struct HeapItem {
        HeapItem( int index, double dist )
            : index( index )
            , dist( dist )
        {
        }
        int index;
        double dist;
        bool operator<( const HeapItem& o ) const
        {
            return dist < o.dist;
        }
    };

    struct DistanceComparator {
        size_t item_idx;
        const Dataset::DataContainer& items;
        DistanceComparator( size_t i, const Dataset::DataContainer& d )
            : item_idx( i )
            , items( d )
        {
        }
        bool operator()( size_t a, size_t b )
        {
            return distance( items[item_idx], items[a] ) < distance( items[item_idx], items[b] );
        }
    };

    uint32_t buildFromPoints( uint32_t lower, uint32_t upper, const Dataset::DataContainer& d )
    {
        if ( upper == lower ) {
            return 0;
        }

        uint32_t cur_node_idx = m_next_idx++;

        Node& node = m_nodelist[cur_node_idx];
        node.index = lower;

        if ( upper - lower > 1 ) {

            int i = (int)( (double)rand() / RAND_MAX * ( upper - lower - 1 ) ) + lower;
            std::swap( m_items_idx[lower], m_items_idx[i] );

            int median = ( upper + lower ) / 2;

            std::nth_element(
                m_items_idx.begin() + lower + 1,
                m_items_idx.begin() + median,
                m_items_idx.begin() + upper,
                DistanceComparator( m_items_idx[lower], d ) );
            node.threshold = distance( d[m_items_idx[lower]], d[m_items_idx[median]] );

            node.index = lower;
            node.left = buildFromPoints( lower + 1, median, d );
            node.right = buildFromPoints( median, upper, d );
        }

        return cur_node_idx;
    }

    void search( uint32_t node_idx, const T& target, TNeighborsList& nlist, double& t, const Dataset::DataContainer& d ) const
    {
        if ( node_idx == 0 )
            return;

        const Node& node = m_nodelist[node_idx];

        const double dist = distance( d[m_items_idx[node.index]], target );

        if ( dist < t ) {
            nlist.push_back( std::make_pair( m_items_idx[node.index], dist ) );
        }

        if ( node.left == 0 && node.right == 0 ) {
            return;
        }

        if ( dist < node.threshold ) {
            if ( dist - t <= node.threshold ) {
                search( node.left, target, nlist, t, d );
            }

            if ( dist + t >= node.threshold ) {
                search( node.right, target, nlist, t, d );
            }

        } else {
            if ( dist + t >= node.threshold ) {
                search( node.right, target, nlist, t, d );
            }

            if ( dist - t <= node.threshold ) {
                search( node.left, target, nlist, t, d );
            }
        }
    }
};
}

#endif // VPTREE_H
