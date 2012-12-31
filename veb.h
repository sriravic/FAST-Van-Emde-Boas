#ifndef __VEB_H__
#define __VEB_H__

#ifdef _MSC_VER
#include <intrin.h>
#include <stdint.h>

#define ulong  unsigned long
#define uint	unsigned int

static ulong __inline ctz( ulong x )
{
   ulong r = 0;
   _BitScanReverse(&r, x);
   return r;
}

static ulong __inline clz( ulong x )
{
   ulong r = 0;
   _BitScanForward(&r, x);
   return r;
}


#endif

static inline int bit_set(uint8_t *bitmap, int bit)
{
    return !!(bitmap[bit >> 3] & (1 << (bit & 7)));
}

static inline void set_bit(uint8_t *bitmap, int bit, int val)
{
    bitmap[bit >> 3] |= (val << (bit & 7));
}

static inline int fls(int f)
{
    int order;
    for (order = 0; f; f >>= 1, order++) ;

    return order;
}

static inline int ilog2(int f)
{
    return fls(f) - 1;
}

static inline int is_power_of_two(int f)
{
    return (f & (f-1)) == 0;
}

static inline int hyperfloor(int f)
{
    return 1 << (fls(f) - 1);
}

static inline int hyperceil(int f)
{
    return 1 << fls(f-1);
}

int bfs_to_veb(int bfs_number, int height)
{
    int split;
    int top_height, bottom_height;
    int depth;
    int subtree_depth, subtree_root, num_subtrees;
    int toptree_size, subtree_size;
    unsigned int mask;
    int prior_length;

    /* if this is a size-3 tree, bfs number is sufficient */
    if (height <= 2)
        return bfs_number;

    /* depth is level of the specific node */
    depth = ilog2(bfs_number);

    /* the vEB layout recursively splits the tree in half */
    split = hyperceil((height + 1) / 2);
    bottom_height = split;
    top_height = height - bottom_height;

    /* node is located in top half - recurse */
    if (depth < top_height)
        return bfs_to_veb(bfs_number, top_height);

    /*
     * Each level adds another bit to the BFS number in the least
     * position.  Thus we can find the subtree root by shifting off
     * depth - top_height rightmost bits.
     */
    subtree_depth = depth - top_height;
    subtree_root = bfs_number >> subtree_depth;

    /*
     * Similarly, the new bfs_number relative to subtree root has
     * the bit pattern representing the subtree root replaced with
     * 1 since it is the new root.  This is equivalent to
     * bfs' = bfs / sr + bfs % sr.
     */

    /* mask off common bits */
    num_subtrees = 1 << top_height;
    bfs_number &= (1 << subtree_depth) - 1;

    /* replace it with one */
    bfs_number |= 1 << subtree_depth;

    /*
     * Now we need to count all the nodes before this one, then the
     * position within this subtree.  The number of siblings before
     * this subtree root in the layout is the bottom k-1 bits of the
     * subtree root.
     */
    subtree_size = (1 << bottom_height) - 1;
    toptree_size = (1 << top_height) - 1;

    prior_length = toptree_size +
        (subtree_root & (num_subtrees - 1)) * subtree_size;

    return prior_length + bfs_to_veb(bfs_number, bottom_height);
}

#endif
