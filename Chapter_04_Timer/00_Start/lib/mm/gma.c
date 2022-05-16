/*! Dynamic memory allocator - Grid Memory Allocator, based on TLSF algorithm */

#define _GMA_C_
#include <lib/gma.h>

static gma_t pool; /* first pool, used if none is specified */

/*!
 * Initialize dynamic memory manager
 * \param memory_segment Memory pool start address
 * \param size Memory pool size
 * \param min_chunk_size Minimal chunk size
 * \param flags Various flags
 * \return memory pool descriptor
 */
gma_t *gma_init(void *memory_segment, size_t size, size_t min_chunk_size,
		  uint flags)
{
	gma_t *mpool;
	size_t addr, end;
	uint levels, i, j;
	void *chunk;

	ASSERT(memory_segment);

	/* usable segment: from 'addr' to 'end' */
	addr = CHUNK_ALIGN_FW(memory_segment);
	end = CHUNK_ALIGN(memory_segment + size);

	ASSERT(end - addr >= MIN_POOL_SIZE);

	if (flags & NEW_MPOOL)
	{
		mpool = (gma_t *) addr;
		addr = CHUNK_ALIGN_FW(addr + sizeof(gma_t));
	}
	else {
		mpool = &pool;
	}

	/* minimal chunk size must be at least large as header for free chunk */
	if (min_chunk_size == 0) /* if not set */
		min_chunk_size = DEF_MIN_CHUNK_SIZE;

	if (min_chunk_size >= MIN_CHUNK_SIZE)
		mpool->min_chunk_size = min_chunk_size;
	else
		mpool->min_chunk_size = MIN_CHUNK_SIZE;

	mpool->fl_min = msb_index(mpool->min_chunk_size);

	mpool->fl_max = msb_index(size);

	levels = mpool->fl_max - mpool->fl_min + 1;

	mpool->SL_bitmap = (size_t *) addr;
	addr = CHUNK_ALIGN_FW(addr + sizeof(size_t) * levels);
	for (i = 0; i < levels; i++)
		mpool->SL_bitmap[i] = 0;

	mpool->chunk = (mchunk_t *(*) [SL_DIM]) addr;
	addr = CHUNK_ALIGN_FW(addr + sizeof(mchunk_t *) * SL_DIM * levels);
	for (i = 0; i < levels; i++)
		for (j = 0; j < SL_DIM; j++)
			mpool->chunk[i][j] = NULL;

	/*
	 * for 'extend' and 'shrink' operations
	 * mpool->pool = memory_segment;
	 * mpool->size = size;
	 */

	/* Create first chunk that occupy whole usable area  */
	chunk = make_first_chunk((void *) addr, end - addr);

	/* "free" chunk */
	gma_free(mpool, chunk);

	return mpool;
}

/*!
 * Memory allocation for chunk of size 'size'
 * \param mpool Memory pool pointer, or NULL (for default)
 * \param size Requested memory chunk size
 * \return allocated chunk address (after header),
           NULL if don't have enough free memory
 */
void *gma_alloc(gma_t *mpool, size_t size)
{
	size_t fl, sl;
	mchunk_t *chunk, *remainder;

	ASSERT(size > 0 && size < MAX_CHUNK_SIZE);

	if (mpool == NULL)
		mpool = &pool;

	size = CHUNK_ALIGN_FW(size + sizeof(size_t)); /* add header size */

	if (size < mpool->min_chunk_size)
		size = mpool->min_chunk_size;

	if (get_indexes(mpool, size, &fl, &sl, 0))
		return NULL;

	if (!(chunk = remove_first_chunk_from_free_list(mpool, fl, sl)))
		return NULL;

	if (GET_CHUNK_SIZE(chunk) >= size + mpool->min_chunk_size)
	{
		remainder = split_chunk_at(chunk, size);

		insert_chunk_in_free_list(mpool, remainder);
	}

	SET_CHUNK_IN_USE(chunk);

	return GET_CHUNK_USABLE_ADDR(chunk);
}

/*!
 * Free chunk
 * \param address Block address (after header - 'usable' memory)
 */
int gma_free(gma_t *mpool, void *address)
{
	mchunk_t *chunk, *before, *after;

	chunk = GET_CHUNK_HDR_FROM_USABLE_ADDR(address);

	ASSERT(address);
	ASSERT(GET_CHUNK_INUSE(chunk));
	ASSERT(!IS_BORDER_CHUNK(chunk));
	ASSERT(CHUNK_IS_ALIGNED(chunk));

	if (mpool == NULL)
		mpool = &pool;

	CLEAR_CHUNK_INUSE(chunk);

	before = GET_CHUNK_BEFORE(chunk);
	if (before && !GET_CHUNK_INUSE(before))
	{
		remove_chunk_from_free_list(mpool, before);
		JOIN_CHUNKS(before, chunk);
		chunk = before;
	}

	after = GET_CHUNK_AFTER(chunk);
	if (after && !GET_CHUNK_INUSE(after))
	{
		remove_chunk_from_free_list(mpool, after);
		JOIN_CHUNKS(chunk, after);
	}

	insert_chunk_in_free_list(mpool, chunk);

	return 0;
}

/*!
 * Return indexes (first and second level) of list where we put free chunk
 * (when 'insert' != 0), or where we start search for free chunk
 * (when 'insert' == 0)
 * \param mpool Memory pool pointer (must not be NULL!)
 * \param size Block size
 * \param fl Address for first level index
 * \param al Address for second level index
 * \param insert Indexes for 'insert' or 'search' operations?
 * \return 0 when successful, -1 when size is out of range
 */
static int get_indexes(gma_t *mpool, size_t size, size_t *fl, size_t *sl,
			 int insert)
{
	size_t bits;

	ASSERT(fl && sl && size >= MIN_CHUNK_SIZE && CHUNK_IS_ALIGNED(size));

	if (!insert && size >= EXACT_LIMIT_SIZE)
		size += (1 << (msb_index(size) - L)) - 1;
		/* if searching for free block, add "little" to size so that we
		   search in right list - see previous short explanation */

	*fl = msb_index(size);

	if (*fl > mpool->fl_max)
	{
		LOG(ERROR, "Requested too large chunk of memory");
		return EXIT_FAILURE;
	}

	*sl = (size >> (*fl - L)) - (1 << L);

	*fl -= mpool->fl_min; /* returned 'fl' is prepared as index */

	if (!insert)
	{
		/* Is calculated list non-empty? Or do we need to search
		   forward, in lists that have larger chunks? */
		bits = mpool->SL_bitmap[*fl] &((~((size_t) 0)) << (*sl));

		if (bits != 0)
		{
			*sl = lsb_index(bits);
		}
		else {
			bits = mpool->FL_bitmap &((~((size_t) 0))<< (*fl + 1));
			if (bits == 0)
				return EXIT_FAILURE;

			*fl = lsb_index(bits);
			*sl = lsb_index(mpool->SL_bitmap[*fl]);
		}
	}

	return 0;
}

static inline void set_list_have_chunks(gma_t *mpool, size_t fl, size_t sl)
{
	mpool->SL_bitmap[fl] |= ((size_t) 1) << sl;
	mpool->FL_bitmap |= ((size_t) 1) << fl;
}

static inline void clear_list_have_chunks(gma_t *mpool, size_t fl, size_t sl)
{
	mpool->SL_bitmap[fl] &= ~(((size_t) 1) << sl);
	if (!mpool->SL_bitmap[fl]) /* all bits are zero? */
		mpool->FL_bitmap &= ~(((size_t) 1) << fl);
}

static void insert_chunk_in_free_list(gma_t *mpool, mchunk_t *chunk)
{
	size_t fl = 0, sl = 0;

	get_indexes(mpool, GET_CHUNK_SIZE(chunk), &fl, &sl, 1);

	insert_chunk_in_list(&mpool->chunk[fl][sl], chunk);

	set_list_have_chunks(mpool, fl, sl);
}

static void remove_chunk_from_free_list(gma_t *mpool, mchunk_t *chunk)
{
	size_t fl = 0, sl = 0;

	if (remove_chunk_from_list(chunk))
	{
		get_indexes(mpool, GET_CHUNK_SIZE(chunk), &fl, &sl, 1);
		clear_list_have_chunks(mpool, fl, sl);
	}
}

static mchunk_t *remove_first_chunk_from_free_list(gma_t *mpool, size_t fl,
						     size_t sl)
{
	mchunk_t *chunk = FIRST_IN_LIST(mpool->chunk[fl][sl]);

	if (remove_chunk_from_list(chunk))
		clear_list_have_chunks(mpool, fl, sl);

	return chunk;
}
