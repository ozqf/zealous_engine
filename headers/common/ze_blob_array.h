#ifndef ZE_BLOB_ARRAY_H
#define ZE_BLOB_ARRAY_H

#include "../ze_common.h"
#include <string.h>

#define ZE_BA_SENTINEL 0xDEADBEEF

#define ZE_BA_INVALID_INDEX -1

#ifndef ZE_BA_SET_ZERO
#define ZE_BA_SET_ZERO(ptrToSet, numBytesToZero) \
    memset(##ptrToSet##, 0, numBytesToZero##)
#endif

#ifndef ZE_BA_COPY
#define ZE_BA_COPY(destBlobPtr, sourceBlobPtr, blobSize) \
    memcpy((void *)##destBlobPtr##, (void *)##sourceBlobPtr##, blobSize##);
#endif

#define ZE_BA_STATUS_FREE 0
#define ZE_BA_STATUS_OCCUPIED 1
#define ZE_BA_STATUS_RECYCLE 2

struct ZEBlobHeader
{
    i32 id;
    // if not marked as valid, this is an 'empty' slot but not available for reassignment yet
    // valid items are slots safe for truncation.
    i32 status;
    // TODO: sentinel is not checked as when GetByIndex runs for a free blob,
    // the sentinel is not set yet!
    i32 sentinel;
};

struct ZEBlobArray
{
    u8 *m_blobs;
    u8 *m_end;
    zeSize m_totalBlobSize;
    zeSize m_blobUserSize;
    i32 m_maxBlobs;
    i32 m_numBlobs;
    i32 m_invalidId;

    ///////////////////////////////////////////////////
    // Get headers
    ///////////////////////////////////////////////////
    ZEBlobHeader *GetHeaderByIndex(i32 index)
    {
        ZE_ASSERT((index >= 0 && index < m_numBlobs), "Blob array out of bounds");
        zeSize offset = m_totalBlobSize * index;
        ZEBlobHeader *result = (ZEBlobHeader *)(m_blobs + offset);
        ZE_ASSERT(result->sentinel == ZE_BA_SENTINEL, "Blob GetByIndex sentinel failed")
        return result;
    }

    ZEBlobHeader *GetHeaderByIndexUnchecked(i32 index)
    {
        zeSize offset = m_totalBlobSize * index;
        ZEBlobHeader *result = (ZEBlobHeader *)(m_blobs + offset);
        return result;
    }

    ///////////////////////////////////////////////////
    // Get users slots
    ///////////////////////////////////////////////////

    u8 *GetByIndexUnchecked(i32 index)
    {
        ZEBlobHeader *h = GetHeaderByIndexUnchecked(index);
        return (u8 *)h + sizeof(ZEBlobHeader);
    }

    u8 *GetByIndex(i32 index)
    {
        //if (m_numBlobs == 0) { return NULL; }
        ZE_ASSERT((index >= 0 && index < m_numBlobs), "Blob array out of bounds");
        zeSize offset = m_totalBlobSize * index;
        ZEBlobHeader *h = GetHeaderByIndexUnchecked(index);
        ZE_ASSERT(h->sentinel == ZE_BA_SENTINEL, "Blob GetByIndex sentinel failed")
        if (h->status != ZE_BA_STATUS_OCCUPIED)
        {
            return NULL;
        }
        return (u8 *)h + sizeof(ZEBlobHeader);
    }

    i32 FindLastOccupiedSlot(i32 i)
    {
        if (i < 0 || i >= m_numBlobs)
        {
            i = m_numBlobs - 1;
        }
        while (i >= 0)
        {
            ZEBlobHeader *h = GetHeaderByIndex(i);
            if (h->status == ZE_BA_STATUS_OCCUPIED)
            {
                return i;
            }
            i--;
        }
        return ZE_BA_INVALID_INDEX;
    }

    /**
     * If data result or index result are null they are ignored
     */
    ErrorCode GetFreeSlot(u8 **dataResult, i32 *indexResult, i32 id)
    {
        if (id == m_invalidId)
        {
            return ZE_ERROR_BAD_ARGUMENT;
        }
        if (m_numBlobs == m_maxBlobs)
        {
            return ZE_ERROR_NO_SPACE;
        }

        i32 newBlobIndex = m_numBlobs++;

        zeSize offset = m_totalBlobSize * newBlobIndex;
        ZEBlobHeader *header = (ZEBlobHeader *)(m_blobs + offset);
        ZE_ASSERT(header->status != ZE_BA_STATUS_OCCUPIED, "Attempting to reassigned in-use blob")
        header->id = id;
        header->sentinel = ZE_BA_SENTINEL;
        header->status = ZE_BA_STATUS_OCCUPIED;
        if (dataResult != NULL)
        {
            *dataResult = (u8 *)header + sizeof(ZEBlobHeader);
        }
        if (indexResult != NULL)
        {
            *indexResult = newBlobIndex;
        }
        return ZE_ERROR_NONE;
    }

    void ClearHeader(ZEBlobHeader *h)
    {
        ZE_ASSERT(h->sentinel == ZE_BA_SENTINEL, "Clear header - sentinel check failed")
        h->id = m_invalidId;
        h->status = ZE_BA_STATUS_FREE;
    }

    ErrorCode MarkForFree(i32 index)
    {
        if (index >= m_numBlobs)
        {
            return ZE_ERROR_OUT_OF_BOUNDS;
        }
        ZEBlobHeader *header = GetHeaderByIndex(index);
        if (header == NULL)
        {
            return ZE_ERROR_NOT_FOUND;
        }
        header->status = ZE_BA_STATUS_RECYCLE;
        return ZE_ERROR_NONE;
    }

    void CopyOver(i32 sourceIndex, i32 destIndex)
    {
        u8 *sourcePtr = m_blobs + (m_totalBlobSize * sourceIndex);
        u8 *destPtr = m_blobs + (m_totalBlobSize * destIndex);
        ZE_BA_COPY(destPtr, sourcePtr, m_totalBlobSize)
    }

    void Truncate()
    {
        i32 i = 0;
        while (i < m_numBlobs)
        {
            // if blob is not valid, replace it with the last in the array
            // and reduce the blob count
            ZEBlobHeader *blob = GetHeaderByIndex(i);
            if (blob->status == ZE_BA_STATUS_OCCUPIED)
            {
                i++;
                continue;
            }

            ZE_ASSERT(blob->status == ZE_BA_STATUS_RECYCLE, "Free blob found during truncate!")

            if (m_numBlobs == 1)
            {
                // no blob to replace this with, just clear list
                ClearHeader(blob);
                m_numBlobs = 0;
                return;
            }
            else
            {
                i32 swapIndex = (m_numBlobs - 1);
                CopyOver(swapIndex, i);
                ClearHeader(GetHeaderByIndexUnchecked(swapIndex));
                m_numBlobs--;
            }
        }
    }
};

static ErrorCode ZE_CreateBlobArray(
    ZE_mallocFunction mallocFn,
    ZEBlobArray **result, i32 capacity, zeSize sizePerObject, i32 invalidId)
{
    zeSize sizePerBlob = sizeof(ZEBlobHeader) + sizePerObject;
    zeSize totalBytes = sizeof(ZEBlobArray) + (sizePerBlob * capacity);
    u8 *mem = (u8*)mallocFn(totalBytes);
    if (mem == NULL)
    {
        return ZE_ERROR_ALLOCATION_FAILED;
    }
    ZEBlobArray *arr = (ZEBlobArray *)mem;
    *arr = {};
    arr->m_blobs = mem + sizeof(ZEBlobArray);
    arr->m_end = mem + totalBytes;
    arr->m_blobUserSize = sizePerObject;
    arr->m_totalBlobSize = sizePerBlob;
    arr->m_numBlobs = 0;
    arr->m_maxBlobs = capacity;
    arr->m_invalidId = invalidId;
    for (i32 i = 0; i < arr->m_maxBlobs; ++i)
    {
        ZEBlobHeader *h = arr->GetHeaderByIndexUnchecked(i);
        h->id = invalidId;
        h->status = ZE_BA_STATUS_FREE;
        // Sentinels should NEVER be changed once set here
        h->sentinel = ZE_BA_SENTINEL;
    }
    *result = arr;
    return ZE_ERROR_NONE;
}

static void ZE_DeleteBlobArray(ZEBlobArray *arr, ZE_freeFunction freeFn)
{
    freeFn(arr);
}

#endif // ZE_BLOB_ARRAY_H