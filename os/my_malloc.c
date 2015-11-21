/*
 * mm.c - mmmalloc - Maurice Moss's Memory Allocator
 * 
 * =================
 * Allocating memory
 * =================
 * 
 * 1. An allocation request arrives
 * 2. It will be sorted into a category based on its size
 *         8 <= n <  64    Allocated in blocks of  512 bytes
 *        64 <= n < 256    Allocated in blocks of 4096 bytes
 *       256 <= n          Allocated on demand
 * 3. Large allocations are stored separately in a red-black tree
 *    ordered by base-address.
 * 4. The allocation will be further sorted into a bin also based on
 *    its size:
 *          8 <= n <    64    Bin sizes are 8 bytes apart
 *                                (  64 -  8) /  8 = 7 bins
 *        64 <= n <   256    Bin sizes are 64 bytes apart
 *                                ( 256 - 64) / 64 = 3 bins
 * 5. For its bin it will retrieve the root of a red-black tree of runs.
 *    Each run is laid out with a 4 byte parent pointer, 1 byte color,
 *    4 byte left pointer, 4 byte right pointer, 2 byte slot size, 1 byte
 *    slot count, and 8 byte bitmap that keeps track of what slots are allocated.
 *
 *     ------------------
 *    |  0 | parent      |
 *    |  4 | color       |
 *    |  5 | left        |
 *    |  9 | right       |
 *    | 13 | slot count  |
 *    | 14 | size        |
 *    | 16 | bitmap      |
 *    | 24 | slot 1      |
 *     ------------------
 *    
 *    The sorting criteria is the run base-addresses.
 *    A linear search is used to find the first free slot.
 *    If a free slot is found then the bitmap is changed to mark
 *    the slot as allocated and the address of the slot is returned.
 * 6. If no empty run is found in the correct bin a new run will have
 *    to be allocated.
 * 7. Free runs are stored in a red-black tree ordered by base-address.
 *    If the tree is empty a new run is allocated.
 * 
 * ==============
 * Freeing memory
 * ==============
 * 
 * 1. A free request arrives.
 * 2. Binary search is used to search in all the bins for a
 *    run that has the correct base address for the allocation.
 * 3. Large allocations will be found in the separate large allocation
 *    tree.
 * 4. The offset is used to calculate the correct bit to toggle in
 *    the run's bitmap.
 * 5. If the run is empty it is removed from its tree and added to
 *    the free-run tree. If the run below it is adjacent then they are
 *    merged, if the run above it is adjacent then they too are merged.
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Maurice_Moss",
    /* First member's full name */
    "Asgeir Bjarni Ingvarsson",
    /* First member's email address */
    "asgeirb09@ru.is",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    "",
    /* Third member's full name (leave blank if none) */
    "",
    /* Third member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* 
 * Allocations below this size will be rounded to
 * the nearest multiple of the platform alignment
 */
#define ALIGNMENT_STEP_LIMIT 64

/*
 * The bin size for alignment spaced allocations
 */
#define ALIGNMENT_RUN 512

/*
 * The distance between bins for small allocations
 */
#define SMALL_STEP 64

/*
 * Allocations below this size will be rounded to
 * the nearest multiple of small step
 */
#define SMALL_STEP_LIMIT 256

/*
 * The bin size for small spaced allocations
 */
#define SMALL_RUN 4096

/*
 * The size of the header needed to keep track of run metadata
 */
#define RUN_HEADER_SIZE 24

/*
 * Numbers of bins for each size category
 */
#define ALIGNMENT_BIN_COUNT ((ALIGNMENT_STEP_LIMIT-ALIGNMENT)/ALIGNMENT)

#define SMALL_BIN_COUNT ((SMALL_STEP_LIMIT-ALIGNMENT_STEP_LIMIT)/SMALL_STEP)

/*
 * Total number of bins
 */
#define BIN_COUNT (ALIGNMENT_BIN_COUNT + SMALL_BIN_COUNT)

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* rounds up to the nearest multiple of a given alignment */
#define ALIGN_CUSTOM(size, alignment)\
                    (((size) + ((alignment)-1)) & ~((alignment)-1))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * Red-black Tree
 * 
 * The red-black tree functionality is courtesy of libtree
 * https://github.com/fbuihuu/libtree
 */

static inline char *rbtree_first(char *tree);
static inline char *rbtree_last(char *tree);
static char *rbtree_next(char *node);
static char *rbtree_prev(char *node);

static inline int rbtree_compare_runs(char *key, char *node);
static inline char *rbtree_get_left(char *node);
static inline char *rbtree_get_right(char *node);

static char *rbtree_lookup(char *key, char *tree);
static char *rbtree_insert(char *node, char **tree);
static void rbtree_remove(char *node, char **tree);
static void rbtree_replace(char *old, char *new, char **tree);
static inline char *rbtree_init(char *tree);

/*
 * Allocator globals
 */

#ifdef MM_CHECK
int mm_check();
void mm_print_bins();
void mm_print_free_runs();
#endif

/*
 * Determines in which bin an allocation of the given size belongs.
 */
static inline int mm_get_binindex(size_t size);
/*
 * How much memory to allocate for a bin with a given slot size.
 */
static inline int mm_get_runsize(size_t size);
/*
 * Determines the correct alignment for an allocation of a given size.
 */
static inline int mm_get_alignment(size_t size);
/*
 * Finds out which tree the run belongs to.
 * Returns the correct tree, the bin index if applicable, and the run.
 */
static char *mm_findnodetree(char *run, char **out_tree, int *out_bin_index);

/*
 * Finds the first, not full, run in the given bin
 */
static inline char *mmbin_get_available(char *bin);

/*
 * If the run is much larger than the requested size split it
 * and return the rest to the free list.
 *
 * Returns the size of the run after splitting.
 */
static inline size_t mmrun_split(size_t size, char *run);
/*
 * Allocates a new run of the given size.
 * Returns the actual allocation size and the allocated run.
 * On error will return 0 and NULL.
 */
static size_t mmrun_allocate(size_t size, char **allocated);
/*
 * Initializes the run header using the given values.
 * Returns the initialized run.
 */
static inline char *mmrun_init(int slot_count, int slot_size, char *run);
/*
 * Returns the total size of the run.
 */
static inline int mmrun_get_size(char *run);
/*
 * Returns the number of slots in the run.
 */
static inline char mmrun_get_slotcount(char *run);
/*
 * Sets the number of slots in the run.
 */
static inline void mmrun_set_slotcount(char slot_count, char *run);
/*
 * Returns the size of each slot in the run.
 */
static inline int16_t mmrun_get_slotsize(char *run);
/*
 * Sets the slot size of the run.
 */
static inline void mmrun_set_slotsize(int16_t size, char *run);
/*
 * Returns the total size of a run that has no slots.
 */
static inline int32_t mmrun_get_largesize(char *run);
/*
 * Sets the total size of a run that has no slots.
 */
static inline void mmrun_set_largesize(int32_t size, char *run);
/*
 * Returns the index of the first free slot in the given run.
 * Returns -1 if the run is full.
 */
static inline int mmrun_get_firstfree(char *run);
/*
 * Toggles the state (allocated/free) of the given slot in the given run.
 */
static inline void mmrun_toggleslot(int slot, char *run);
/*
 * Returns true if the run has no allocated slots.
 */
static inline int mmrun_isempty(char *run);

/*
 * Adds the given run to the free list.
 */
static void mmfreerun_add(char *run);

/* The bins that hold the packed allocations */
static char **bins;
/* The free run list */
static char *free_runs;
/* The large allocation list */
static char *large_allocations;

/*--------------------------------------------------------------------------*
   Global allocator functions
 *--------------------------------------------------------------------------*/

/* 
 * initialize the malloc package.
 */
int mm_init(void)
{
    int i;
    
    /* Initialize the bin list */
    bins = (char **)mem_sbrk(ALIGN(BIN_COUNT * sizeof(char *)));
    if (bins == (char **)-1) {
        fprintf(stderr, "Unable to allocate space for bins.");
        return !0;
    }
    for (i = 0; i < BIN_COUNT; ++i) {
        bins[i] = NULL;
    }
    
    /* Initialize the large allocation tree */
    large_allocations = NULL;
    
    /* Initialize the free run list */
    free_runs = NULL;
    
    return 0;
}

/* 
 * Allocates at least size bytes and returns a pointer to the allocation.
 * Returns NULL on error.
 */
void *mm_malloc(size_t size)
{
    int bin_index;
    int slot_index;
    char *run;
    
    /* Escape early from invalid requests */
    if (size == 0) {
        return NULL;
    }
    
    /*
     * Align the size given its category and find out what bin
     * it belongs to.
     */
    size = ALIGN_CUSTOM(size, mm_get_alignment(size));
    bin_index = mm_get_binindex(size);
    if (bin_index >= 0) {
        /* Tiny or small allocation */
        
        /* Find out how large the run needs to be */
        int run_size = mm_get_runsize(size);
        /* Find out how many slots it can hold */
        int slots = (run_size - RUN_HEADER_SIZE) / size;
        if ((run_size - ((slots + 1) * (int)size)) < 0) {
            slots -= 1;
        }

        /* If there are no runs of the correct size allocate a new one */
        if (bins[bin_index] == NULL) {
            mmrun_allocate(run_size, &bins[bin_index]);
            if (bins[bin_index] == NULL) {
                /* Out of memory */
                return NULL;
            }
            
            mmrun_init(slots, size, bins[bin_index]);
        }
        
        /*
         * Find a run with a free slot.
         * If no run is found allocate a new one and add it to the bin.
         */
        run = mmbin_get_available(bins[bin_index]);
        if (run == NULL) {
            mmrun_allocate(run_size, &run);
            if (run == NULL) {
                /* Out of memory */
                return NULL;
            }
            
            mmrun_init(slots, size, run);
            rbtree_insert(run, &bins[bin_index]);
        }
        
        /* Find the first free slot */
        slot_index = mmrun_get_firstfree(run);
        if (slot_index < 0) {
            fprintf(stderr, "Available run doesn't have any free slots\n");
            return NULL;
        }
        
        /* Allocate the slot */
        mmrun_toggleslot(slot_index, run);
        
        /* Return the slot's address */
        char *slot_address = (run + mm_get_runsize(size) -
                              ((slot_index+1) * mmrun_get_slotsize(run)));
        return slot_address;
    }
    
    /* large allocation */
    /* Reserve space for the run header and align the size. */
    size = ALIGN(size + RUN_HEADER_SIZE);
    char *chunk;
    
    /* Allocate a new run */
    int chunk_size = mmrun_allocate(size, &chunk);
    if (chunk == NULL) {
        /* Out of memory */
        return NULL;
    }
    
    /* Initialize its metadata and add it to the large allocation list. */
    mmrun_init(0, 0, chunk);
    mmrun_set_largesize(chunk_size, chunk);
    rbtree_insert(chunk, &large_allocations);
    
    /* Return the address of the allocated run */
    return chunk + RUN_HEADER_SIZE;
}

/*
 * Frees the memory pointed to by ptr.
 */
void mm_free(void *ptr)
{
    int bin_index;
    char *tree;
    char *slot_ptr = (char *)ptr;

    /* Find out which tree the pointer belongs to */
    char *run = mm_findnodetree(slot_ptr, &tree, &bin_index);
    if (!run) {
        fprintf(stderr, "Trying to free an invalid pointer.\n");
        return;
    }
    
    if (tree != large_allocations) {
        /* Tiny or small allocation */
        
        /* Free the slot */
        int32_t pos = ((int32_t)run + mmrun_get_size(run) - (int32_t)ptr);
        mmrun_toggleslot((pos / mmrun_get_slotsize(run)) - 1, run);
        
        /* Free the run if it is empty */
        if (mmrun_isempty(run)) {
            rbtree_remove(run, &bins[bin_index]);
            mmfreerun_add(run);
        }
    } else {
        /* Remove the run from the large allocation list and free it */
        rbtree_remove(run, &large_allocations);
        mmfreerun_add(run);
    }
}

/*
 * Change the size of an allocation and copy data from the old pointer.
 *
 * If ptr is NULL mm_reallloc is the same as mm_malloc.
 * If size is 0 mm_realloc is the same as mm_free.
 *
 * Returns a pointer to the newly allocated memory.
 */
void *mm_realloc(void *ptr, size_t size)
{
    /* If size is 0 free the pointer */
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    
    /* If ptr is NULL just allocate */
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    
    /* Find the run the old pointer belongs to */
    char *old_run = mm_findnodetree((char *)ptr, NULL, NULL);
    
    /* Find out the size of the old pointer */
    int old_size;
    if (mmrun_get_slotcount(old_run)) {
        old_size = mmrun_get_slotsize(old_run);
    } else {
        old_size = mmrun_get_largesize(old_run);
    }
    
    /* See if ptr can be expanded */
    if (mmrun_get_slotcount(old_run) == 0) {
        /* Return if the run is already large enough */
        if ((old_size - RUN_HEADER_SIZE) >= size) {
            return ptr;
        }
        
        /* See if there is a free run after the old run */
        if (free_runs) {
            char *run = rbtree_lookup(old_run + old_size + 1, free_runs);
            
            /* Check if it the expanded run can contain the new size */
            if (run && (mmrun_get_size(run) + old_size) >= size) {
                /* Remove the free run from the free list */
                rbtree_remove(run, &free_runs);
                int run_size = mmrun_get_largesize(run);
                
                /* Merge it with the old run */
                mmrun_init(0, 0, old_run);
                mmrun_set_largesize(old_size + run_size, old_run);
                
                /* Split off any excess */
                mmrun_split(size + RUN_HEADER_SIZE, old_run);
                
                /* Return the expanded run */
                return old_run + RUN_HEADER_SIZE;
            }
        }
    }
    
    /* If ptr can't be expanded just allocate a new run and copy */
    void *new_ptr = mm_malloc(size);
    
    if (new_ptr) {
        /* Copy data from the old pointer to the new one */
        int min_size = (size > old_size) ? old_size : size;
        memcpy(new_ptr, ptr, min_size);
    
        /* Free the old pointer */
        mm_free(ptr);
    }
    
    return new_ptr;
}

#ifdef DEBUG
/*
 * Utility function for tree visualisation.
 * Use gdb to call it.
 */
void mm_print_tree(char *tree)
{
    if (!tree) {
        printf("Empty\n");
        return;
    }
    
    char *node = rbtree_first(tree);
    while (node) {
        printf("%p ", node);
        node = rbtree_next(node);
    }
    
    printf("\n");
}

/*
 * Utility function for bin visualisation.
 * Use gdb to call it.
 */
void mm_print_bins()
{
    int i;
    
    printf("Bins:\n");
    for (i = 0; i < BIN_COUNT; ++i) {
        if (bins[i]) {
            printf("Bin %d: ", i);
            mm_print_tree(bins[i]);
        }
    }
}

/*
 * Utility function for free list visualisation.
 * Use gdb to call it.
 */
void mm_print_free_runs()
{
    printf("Free runs:\n");
    if (free_runs) {
        mm_print_tree(free_runs);
    }
}

/*
 * Returns true if the given tree, along with all its children,
 * lies entirely within the heap.
 */
int mm_tree_ok(char *node)
{
    if (node == NULL) {
        return !0;
    }
    
    if (node < (char *)mem_heap_lo() || node > (char *)mem_heap_hi()) {
        return 0;
    }
    
    if (!mm_tree_ok(rbtree_get_left(node))) {
        return 0;
    }
    
    if (!mm_tree_ok(rbtree_get_right(node))) {
        return 0;
    }
    
    return !0;
}

/*
 * Returns true if any node in tree1 intersects with any node from tree2.
 */
int mm_trees_intersect(char *tree1, char *tree2)
{
    char *node_a, *node_b;
    
    node_a = rbtree_first(tree1);
    while (node_a) {
        node_b = rbtree_first(tree2);
        while (node_b) {
            /* Don't compare nodes to themselves */
            if (node_a == node_b) {
                node_b = rbtree_next(node_b);
                continue;
            }
            
            /* If the runs are equal they intersect */
            if (rbtree_compare_runs(node_b, node_a) == 0 ||
                rbtree_compare_runs(node_a, node_b) == 0) {
                fprintf(stderr, "(%p, %p) ", node_a, node_b);
                return !0;
            }
            node_b = rbtree_next(node_b);
        }
        node_a = rbtree_next(node_a);
    }
    
    return 0;
}

#ifdef MM_CHECK
int mm_check()
{
    int i;
    int j;
    
    /* Check if all nodes in all bins are on the heap */
    for (i = 0; i < BIN_COUNT; ++i) {
        if (bins[i] == NULL) {
            continue;
        }
        
        if (!mm_tree_ok(bins[i])) {
            fprintf(stderr, "Tree in bin %d is not ok.\n", i);
            return 0;
        }
    }

    /* Check if all nodes in the large allocation tree are on the heap */
    if (large_allocations && !mm_tree_ok(large_allocations)) {
        fprintf(stderr, "Large allocation tree is not ok.\n");
        return 0;
    }
    
    /* Check if all free runs are on the heap */
    if (free_runs && !mm_tree_ok(free_runs)) {
        fprintf(stderr, "Free runs tree is not ok.\n");
        return 0;
    }
    
    /*
     * Check if any bins intersect another bin,
     * a large allocation or a free run
     */
    for (i = 0; i < BIN_COUNT; ++i) {
        if (bins[i] == NULL) {
            continue;
        }
        
        for (j = 0; j < BIN_COUNT; ++j) {
            if (j == i || bins[j] == NULL) {
                continue;
            }
            
            if (mm_trees_intersect(bins[i], bins[j])) {
                fprintf(stderr, " Intersection in bins (%d, %d)\n", i, j);
                return 0;
            }
        }
        
        if (large_allocations) {
            if (mm_trees_intersect(bins[i], large_allocations)) {
                fprintf(stderr,
                        " Intersection of bin %d and large allocation\n", i);
                return 0;
            }
        }
        
        if (free_runs) {
            if (mm_trees_intersect(bins[i], free_runs)) {
                fprintf(stderr,
                        " Intersection of bin %d and free run\n", i);
                return 0;
            }
        }
    }
    
    /* Check if any large allocations intersect each other or a free run */
    if (large_allocations) {
        if (mm_trees_intersect(large_allocations, large_allocations)) {
            fprintf(stderr, "Two large allocations intersect.\n");
            return 0;
        }
        
        if (free_runs && mm_trees_intersect(free_runs, free_runs)) {
            fprintf(stderr, "Intersection of large allocation and free run.\n");
            return 0;
        }
    }
    
    /* Check if any free runs intersect */
    if (free_runs) {
        if (mm_trees_intersect(free_runs, free_runs)) {
            fprintf(stderr, "Two free runs intersect.\n");
            return 0;
        }
    }
    
    return !0;
}
#endif /* MM_CHECK */
#endif /* DEBUG */

static inline int mm_get_binindex(size_t size)
{
    if (size < ALIGNMENT_STEP_LIMIT) {
        return (size - ALIGNMENT) / ALIGNMENT;
    } else if (size < SMALL_STEP_LIMIT) {
        return (ALIGNMENT_BIN_COUNT +
                ((size - ALIGNMENT_STEP_LIMIT) / SMALL_STEP));
    }
    
    return -1;
}

static inline int mm_get_runsize(size_t size)
{
    if (size < ALIGNMENT_STEP_LIMIT) {
        return ALIGNMENT_RUN;
    } else if (size < SMALL_STEP_LIMIT) {
        return SMALL_RUN;
    }
    
    return 0;
}

static inline int mm_get_alignment(size_t size)
{
    if (size < ALIGNMENT_STEP_LIMIT) {
        return ALIGNMENT;
    } else if (size < SMALL_STEP_LIMIT) {
        return SMALL_STEP;
    }
    
    return 1;
}

static char *mm_findnodetree(char *run, char **out_tree, int *out_bin_index)
{    
    char *node;
    
    /* See if the run belongs to any bin */
    int i;
    for (i = 0; i < BIN_COUNT; ++i) {
        if (bins[i] != NULL) {
            node = rbtree_lookup(run, bins[i]);
            if (node) {
                if (out_tree) {
                    *out_tree = bins[i];
                }
                if (out_bin_index) {
                    *out_bin_index = i;
                }
                return node;
            }
        }
    }
    
    /* See if the run is a large allocation */
    if (large_allocations && (node = rbtree_lookup(run, large_allocations))) {
        if (out_tree) {
            *out_tree = large_allocations;
        }    
        if (out_bin_index) {
            *out_bin_index = -1;
        }
        return node;
    }
    
    /* The run doesn't belong to any tree */
    if (out_tree) {
        *out_tree = NULL;
    }    
    if (out_bin_index) {
        *out_bin_index = -1;
    }
    return NULL;
}

/*--------------------------------------------------------------------------*
   Run functions
 *--------------------------------------------------------------------------*/
 
static inline char mmrun_get_slotcount(char *run)
{
    return *((int16_t *)(run+13));
}

static inline void mmrun_set_slotcount(char slot_count, char *run)
{
    *((int16_t *)(run+13)) = slot_count;
}

static inline int16_t mmrun_get_slotsize(char *run)
{
    return *((int16_t *)(run+14));
}

static inline void mmrun_set_slotsize(int16_t size, char *run)
{
    *((int16_t *)(run+14)) = size;
}

static inline int mmrun_get_size(char *run)
{
    if (mmrun_get_slotcount(run)) {
        return mm_get_runsize(mmrun_get_slotsize(run));
    } else {
        return mmrun_get_largesize(run);
    }
}

static inline int64_t mmrun_get_bitmap(char *run)
{
    return *((int64_t *)(run+16));
}

static inline void mmrun_set_bitmap(int64_t bitmap, char *run)
{
    *((int64_t *)(run+16)) = bitmap;
}

static inline int32_t mmrun_get_largesize(char *run)
{
    return *((int32_t *)(run+16));
}

static inline void mmrun_set_largesize(int32_t size, char *run)
{
    *((int32_t *)(run+16)) = size;
}

static inline int mmrun_get_firstfree(char *run)
{
    /* Return the index of the most significant high bit */
    int64_t bitmap = mmrun_get_bitmap(run);
    unsigned int first_free = -1;
    
    while (bitmap != 0) {
        ++first_free;
        bitmap = bitmap >> 1;
    }
    
    return first_free;
}

static inline void mmrun_toggleslot(int slot, char *run)
{
    mmrun_set_bitmap(mmrun_get_bitmap(run) ^ (1 << slot), run);
}

static inline int mmrun_isempty(char *run)
{
    int slot_count = mmrun_get_slotcount(run);
    return mmrun_get_bitmap(run) == ((1 << slot_count) - 1);
}

static inline size_t mmrun_split(size_t size, char *run)
{
    int run_size = mmrun_get_largesize(run);
    
    if (run_size - size >= ALIGNMENT_RUN) {
        int new_run_size = run_size - size;
        char *new_run = run + run_size - new_run_size;
        
        /* Initialize the split run */
        mmrun_init(0, 0, new_run);
        mmrun_set_largesize(new_run_size, new_run);
        
        /* Adjust the size of the allocated run */
        run_size = run_size - new_run_size;
        mmrun_init(0, 0, run);
        mmrun_set_largesize(run_size, run);
        
        /* Return the split run to the free list */
        mmfreerun_add(new_run);
    }
    
    return run_size;
}

/*
 * Tries to allocate a suitably sized run from the free list.
 *
 * Returns the size of the allocated run and a pointer to it.
 * Returns 0 and NULL if no suitably sized run can be found.
 */
static size_t mmrun_allocate_freerun(size_t size, char **allocated)
{
    /* Nothing to do if the free list is empty */
    if (free_runs) {
        /*
         * Scan the free list linearly for a run that is of
         * the correct size (address ordered first-fit)
         */
        int run_size;
        char *run = rbtree_first(free_runs);
        while (run && mmrun_get_largesize(run) < size) {
            run = rbtree_next(run);
        }
        
        /* If no run is found return NULL */
        if (!run) {
            *allocated = NULL;
            return 0;
        }
        
        /* Remove the run from the free list */
        rbtree_remove(run, &free_runs);
        run_size = mmrun_get_largesize(run);
        
        run_size = mmrun_split(size, run);
        
        *allocated = run;
        return run_size;
    }
    
    *allocated = NULL;
    return 0;
}

static size_t mmrun_allocate(size_t size, char **allocated)
{
    /* Try to allocate the run from the free list */
    size_t allocated_size = mmrun_allocate_freerun(size, allocated);
    if (!allocated_size) {
        /*
         * The free list is empty or no run of a proper size was found.
         * In that case expand the heap by the requested size.
         */
        char *new_chunk = mem_sbrk(size);
        if (new_chunk == (void *)-1) {
            /* Out of memory */
            *allocated = NULL;
            return 0;
        }
        
        /* Initialize the run */
        mmrun_init(0, 0, new_chunk);
        mmrun_set_largesize(size, new_chunk);
        
        *allocated = new_chunk;
        return size;
    }
    
    return allocated_size;
}

static inline char *mmrun_init(int slot_count, int slot_size, char *run)
{
    rbtree_init(run);
    mmrun_set_bitmap((1 << slot_count) - 1, run);
    mmrun_set_slotcount(slot_count, run);
    mmrun_set_slotsize(slot_size, run);
    
    return run;
}

/*--------------------------------------------------------------------------*
   Bin functions
 *--------------------------------------------------------------------------*/

static inline char *mmbin_get_available(char *bin)
{
    char *run = rbtree_first(bin);
    
    /* Bitmap will be zero for any full run */
    while (run && !mmrun_get_bitmap(run)) {
        run = rbtree_next(run);
    }
    
    return run;
}

/*--------------------------------------------------------------------------*
   Free Run functions
 *--------------------------------------------------------------------------*/

static void mmfreerun_add(char *run)
{
    char *buddy;
    int run_size = mmrun_get_size(run);
    
    /* See if the run can be appended to an adjacent run on the free list */
    buddy = rbtree_lookup((char *)((int)run-1), free_runs);
    if (buddy) {
        int buddy_size = mmrun_get_largesize(buddy);
        
        /* Increase the size of the run on the free list */
        mmrun_set_largesize(buddy_size + run_size, buddy);
        
        return;
    }
    
    /*
     * See if the run can be expanded with an adjacent run
     * from the free list
     */
    buddy = rbtree_lookup((char *)((int)run+run_size+1), free_runs);
    if (buddy) {
        int buddy_size = mmrun_get_largesize(buddy);
        
        /*
         * Remove the old run and add it's size to the new run.
         * Then add the new run to the free list.
         */
        rbtree_remove(buddy, &free_runs);
        mmrun_init(0, 0, run);
        mmrun_set_largesize(buddy_size + run_size, run);
        rbtree_insert(run, &free_runs);
        
        return;
    }
    
    /* The run can't be merged so add it to the free list */
    mmrun_init(0, 0, run);
    mmrun_set_largesize(run_size, run);
        
    rbtree_insert(run, &free_runs);
}

/*--------------------------------------------------------------------------*
   Red-black tree functions
 *--------------------------------------------------------------------------*/

#define RB_RED ((int)1)
#define RB_BLACK ((int)2)

static inline char *rb_get_parent(char *node)
{
    return *((char **)node);
}
static inline void rb_set_parent(char *parent, char *node)
{
    *((char **)node) = parent;
}

static inline char *rbtree_get_left(char *node)
{
    return *((char **)(node+5));
}
static inline void rb_set_left(char *left, char *node)
{
    *((char **)(node+5)) = left;
}

static inline char *rbtree_get_right(char *node)
{
    return *((char **)(node+9));
}
static inline void rb_set_right(char *right, char *node)
{
    *((char **)(node+9)) = right;
}

static inline int rb_get_color(char *node)
{
    return *(node+4);
}
static inline void rb_set_color(int color, char *node)
{
    *(node+4) = (char)color;
}

static inline int rb_is_root(char *node)
{
    return rb_get_parent(node) == NULL;
}

static inline int rb_is_black(char *node)
{
    return rb_get_color(node) == RB_BLACK;
}

static inline int rb_is_red(char *node)
{
    return !rb_is_black(node);
}

static inline char *rbtree_first(char *tree)
{
    while (rbtree_get_left(tree))
        tree = rbtree_get_left(tree);
    return tree;
}

static inline char *rbtree_last(char *tree)
{
    while (rbtree_get_right(tree))
        tree = rbtree_get_right(tree);
    return tree;
}

static char *rbtree_next(char *node)
{
    char *parent;

    if (rbtree_get_right(node))
        return rbtree_first(rbtree_get_right(node));

    while ((parent = rb_get_parent(node)) && rbtree_get_right(parent) == node)
        node = parent;
    return parent;
}

static char *rbtree_prev(char *node)    
{
    char *parent;

    if (rbtree_get_left(node))
        return rbtree_last(rbtree_get_left(node));

    while ((parent = rb_get_parent(node)) && rbtree_get_left(parent) == node)
        node = parent;
    return parent;
}

static inline int rbtree_compare_runs(char *key, char *node)
{
    int32_t delta = (int32_t)key - (int32_t)node;
    if (delta == 0) {
        return 0;
    }
    
    if (delta < 0) {
        return 1;
    }
    
    if (delta < mmrun_get_size(node)) {
        return 0;
    }
    
    return -1;
}

/*
 * 'pparent' and 'is_left' are only used for insertions. Normally GCC
 * will notice this and get rid of them for lookups.
 */
static inline char *rb_do_lookup(char *key, char *tree, char **pparent,
                                 int *is_left)
{
    char *node = tree;

    *pparent = NULL;
    *is_left = 0;

    while (node) {
        int res = rbtree_compare_runs(key, node);
        if (res == 0)
            return node;
        *pparent = node;
        if ((*is_left = res > 0))
            node = rbtree_get_left(node);
        else
            node = rbtree_get_right(node);
    }
    return NULL;
}

/*
 * Rotate operations (They preserve the binary search tree property,
 * assuming that the keys are unique).
 */
static void rb_rotate_left(char *node, char **tree) 
{
    char *p = node;
    char *q = rbtree_get_right(node); /* can't be NULL */
    char *parent = rb_get_parent(p);

    if (!rb_is_root(p)) {
        if (rbtree_get_left(parent) == p)
            rb_set_left(q, parent);
        else
            rb_set_right(q, parent);
    } else
        *tree = q;
    rb_set_parent(parent, q);
    rb_set_parent(q, p);

    rb_set_right(rbtree_get_left(q), p);
    if (rbtree_get_right(p))
        rb_set_parent(p, rbtree_get_right(p));
    rb_set_left(p, q);
}

static void rb_rotate_right(char *node, char **tree)
{
    char *p = node;
    char *q = rbtree_get_left(node); /* can't be NULL */
    char *parent = rb_get_parent(p);

    if (!rb_is_root(p)) {
        if (rbtree_get_left(parent) == p)
            rb_set_left(q, parent);
        else
            rb_set_right(q, parent);
    } else
        *tree = q;
    rb_set_parent(parent, q);
    rb_set_parent(q, p);

    rb_set_left(rbtree_get_right(q), p);
    if (rbtree_get_left(p))
        rb_set_parent(p, rbtree_get_left(p));
    rb_set_right(p, q);
}

static char *rbtree_lookup(char *key, char *tree)
{
    char *parent;
    int is_left;

    return rb_do_lookup(key, tree, &parent, &is_left);
}

static void rb_set_child(char *child, char *node, int left)
{
    if (left)
        rb_set_left(child, node);
    else
        rb_set_right(child, node);
}

static char *rbtree_insert(char *node, char **tree)
{
    char *key, *parent;
    int is_left;

    key = rb_do_lookup(node, *tree, &parent, &is_left);
    if (key)
        return key;

    rb_set_left(NULL, node);
    rb_set_right(NULL, node);
    rb_set_color(RB_RED, node);
    rb_set_parent(parent, node);

    if (parent) {
        rb_set_child(node, parent, is_left);
    } else {
        *tree = node;
    }

    /*
     * Fixup the modified tree by recoloring nodes and performing
     * rotations (2 at most) hence the red-black tree properties are
     * preserved.
     */
    while ((parent = rb_get_parent(node)) && rb_is_red(parent)) {
        char *grandpa = rb_get_parent(parent);

        if (parent == rbtree_get_left(grandpa)) {
            char *uncle = rbtree_get_right(grandpa);

            if (uncle && rb_is_red(uncle)) {
                rb_set_color(RB_BLACK, parent);
                rb_set_color(RB_BLACK, uncle);
                rb_set_color(RB_RED, grandpa);
                node = grandpa;
            } else {
                if (node == rbtree_get_right(parent)) {
                    rb_rotate_left(parent, tree);
                    node = parent;
                    parent = rb_get_parent(node);
                }
                rb_set_color(RB_BLACK, parent);
                rb_set_color(RB_RED, grandpa);
                rb_rotate_right(grandpa, tree);
            }
        } else {
            char *uncle = rbtree_get_left(grandpa);

            if (uncle && rb_is_red(uncle)) {
                rb_set_color(RB_BLACK, parent);
                rb_set_color(RB_BLACK, uncle);
                rb_set_color(RB_RED, grandpa);
                node = grandpa;
            } else {
                if (node == rbtree_get_left(parent)) {
                    rb_rotate_right(parent, tree);
                    node = parent;
                    parent = rb_get_parent(node);
                }
                rb_set_color(RB_BLACK, parent);
                rb_set_color(RB_RED, grandpa);
                rb_rotate_left(grandpa, tree);
            }
        }
    }
    rb_set_color(RB_BLACK, *tree);
    return NULL;
}

static void rbtree_remove(char *node, char **tree)
{
    char *parent = rb_get_parent(node);
    char *left = rbtree_get_left(node);
    char *right = rbtree_get_right(node);
    char *next;
    int color;

    if (!left)
        next = right;
    else if (!right)
        next = left;
    else
        next = rbtree_first(right);

    if (parent)
        rb_set_child(next, parent, rbtree_get_left(parent) == node);
    else
        *tree = next;

    if (left && right) {
        color = rb_get_color(next);
        rb_set_color(rb_get_color(node), next);

        rb_set_left(left, next);
        rb_set_parent(next, left);

        if (next != right) {
            parent = rb_get_parent(next);
            rb_set_parent(rb_get_parent(node), next);

            node = rbtree_get_right(next);
            rb_set_left(node, parent);

            rb_set_right(right, next);
            rb_set_parent(next, right);
        } else {
            rb_set_parent(parent, next);
            parent = next;
            node = rbtree_get_right(next);
        }
    } else {
        color = rb_get_color(node);
        node = next;
    }
    /*
     * 'node' is now the sole successor's child and 'parent' its
     * new parent (since the successor can have been moved).
     */
    if (node)
        rb_set_parent(parent, node);

    /*
     * The 'easy' cases.
     */
    if (color == RB_RED)
        return;
    if (node && rb_is_red(node)) {
        rb_set_color(RB_BLACK, node);
        return;
    }

    do {
        if (node == *tree)
            break;

        if (node == rbtree_get_left(parent)) {
            char *sibling = rbtree_get_right(parent);

            if (rb_is_red(sibling)) {
                rb_set_color(RB_BLACK, sibling);
                rb_set_color(RB_RED, parent);
                rb_rotate_left(parent, tree);
                sibling = rbtree_get_right(parent);
            }
            if ((!rbtree_get_left(sibling)  || rb_is_black(rbtree_get_left(sibling))) &&
                (!rbtree_get_right(sibling) || rb_is_black(rbtree_get_right(sibling)))) {
                rb_set_color(RB_RED, sibling);
                node = parent;
                parent = rb_get_parent(parent);
                continue;
            }
            if (!rbtree_get_right(sibling) || rb_is_black(rbtree_get_right(sibling))) {
                rb_set_color(RB_BLACK, rbtree_get_left(sibling));
                rb_set_color(RB_RED, sibling);
                rb_rotate_right(sibling, tree);
                sibling = rbtree_get_right(parent);
            }
            rb_set_color(rb_get_color(parent), sibling);
            rb_set_color(RB_BLACK, parent);
            rb_set_color(RB_BLACK, rbtree_get_right(sibling));
            rb_rotate_left(parent, tree);
            node = *tree;
            break;
        } else {
            char *sibling = rbtree_get_left(parent);

            if (rb_is_red(sibling)) {
                rb_set_color(RB_BLACK, sibling);
                rb_set_color(RB_RED, parent);
                rb_rotate_right(parent, tree);
                sibling = rbtree_get_left(parent);
            }
            if ((!rbtree_get_left(sibling)  || rb_is_black(rbtree_get_left(sibling))) &&
                (!rbtree_get_right(sibling) || rb_is_black(rbtree_get_right(sibling)))) {
                rb_set_color(RB_RED, sibling);
                node = parent;
                parent = rb_get_parent(parent);
                continue;
            }
            if (!rbtree_get_left(sibling) || rb_is_black(rbtree_get_left(sibling))) {
                rb_set_color(RB_BLACK, rbtree_get_right(sibling));
                rb_set_color(RB_RED, sibling);
                rb_rotate_left(sibling, tree);
                sibling = rbtree_get_left(parent);
            }
            rb_set_color(rb_get_color(parent), sibling);
            rb_set_color(RB_BLACK, parent);
            rb_set_color(RB_BLACK, rbtree_get_left(sibling));
            rb_rotate_right(parent, tree);
            node = *tree;
            break;
        }
    } while (rb_is_black(node));

    if (node)
        rb_set_color(RB_BLACK, node);
}

static void rbtree_replace(char *old, char *new, char **tree)
{
    char *parent = rb_get_parent(old);

    if (parent)
        rb_set_child(parent, new, rbtree_get_left(parent) == old);
    else
        *tree = new;

    if (rbtree_get_left(old))
        rb_set_parent(new, rbtree_get_left(old));
    if (rbtree_get_right(old))
        rb_set_parent(new, rbtree_get_right(old));

    *new = *old;
}

static inline char *rbtree_init(char *tree)
{
    rb_set_parent(NULL, tree);
    rb_set_left(NULL, tree);
    rb_set_right(NULL, tree);
    rb_set_color(RB_BLACK, tree);

    return tree;
}
