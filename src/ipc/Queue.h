/*
 * $Id$
 *
 */

#ifndef SQUID_IPC_QUEUE_H
#define SQUID_IPC_QUEUE_H

#include "Array.h"
#include "ipc/AtomicWord.h"
#include "ipc/mem/Segment.h"
#include "util.h"

class String;

/// Lockless fixed-capacity queue for a single writer and a single reader.
class OneToOneUniQueue {
public:
    class Empty {};
    class Full {};
    class ItemTooLarge {};

    OneToOneUniQueue(const String &id, const unsigned int maxItemSize, const int capacity);
    OneToOneUniQueue(const String &id);

    unsigned int maxItemSize() const { return shared->theMaxItemSize; }
    int size() const { return shared->theSize; }
    int capacity() const { return shared->theCapacity; }

    bool empty() const { return !shared->theSize; }
    bool full() const { return shared->theSize == shared->theCapacity; }

    static int Bytes2Items(const unsigned int maxItemSize, int size);
    static int Items2Bytes(const unsigned int maxItemSize, const int size);

    template <class Value>
    bool pop(Value &value); ///< returns true iff the queue was full
    template <class Value>
    bool push(const Value &value); ///< returns true iff the queue was empty

private:
    struct Shared {
        Shared(const unsigned int aMaxItemSize, const int aCapacity);

        unsigned int theIn; ///< input index, used only in push()
        unsigned int theOut; ///< output index, used only in pop()

        AtomicWord theSize; ///< number of items in the queue
        const unsigned int theMaxItemSize; ///< maximum item size
        const int theCapacity; ///< maximum number of items, i.e. theBuffer size

        char theBuffer[];
    };

    Ipc::Mem::Segment shm; ///< shared memory segment
    Shared *shared; ///< pointer to shared memory
};

/// Lockless fixed-capacity bidirectional queue for two processes.
class OneToOneBiQueue {
public:
    typedef OneToOneUniQueue::Empty Empty;
    typedef OneToOneUniQueue::Full Full;
    typedef OneToOneUniQueue::ItemTooLarge ItemTooLarge;

    /// Create a new shared queue.
    OneToOneBiQueue(const String &id, const unsigned int maxItemSize, const int capacity);
    OneToOneBiQueue(const String &id); ///< Attach to existing shared queue.

    template <class Value>
    bool pop(Value &value) { return popQueue->pop(value); }
    template <class Value>
    bool push(const Value &value) { return pushQueue->push(value); }

private:
    OneToOneUniQueue *const popQueue; ///< queue to pop from for this process
    OneToOneUniQueue *const pushQueue; ///< queue to push to for this process
};

/**
 * Lockless fixed-capacity bidirectional queue for a limited number
 * pricesses. Implements a star topology: Many worker processes
 * communicate with the one central process. The central process uses
 * FewToOneBiQueue object, while workers use OneToOneBiQueue objects
 * created with the Attach() method. Each worker has a unique integer
 * ID in [0, workerCount) range.
 */
class FewToOneBiQueue {
public:
    typedef OneToOneBiQueue::Empty Empty;
    typedef OneToOneBiQueue::Full Full;
    typedef OneToOneBiQueue::ItemTooLarge ItemTooLarge;

    FewToOneBiQueue(const String &id, const int aWorkerCount, const unsigned int maxItemSize, const int capacity);
    static OneToOneBiQueue *Attach(const String &id, const int workerId);
    ~FewToOneBiQueue();

    bool validWorkerId(const int workerId) const;
    int workerCount() const { return theWorkerCount; }

    template <class Value>
    bool pop(int &workerId, Value &value); ///< returns false iff the queue was full
    template <class Value>
    bool push(const int workerId, const Value &value); ///< returns false iff the queue was empty

private:
    int theLastPopWorkerId; ///< the last worker ID we pop()ed from
    Vector<OneToOneBiQueue *> biQueues; ///< worker queues
    const int theWorkerCount; ///< number of worker processes
};


// OneToOneUniQueue

template <class Value>
bool
OneToOneUniQueue::pop(Value &value)
{
    if (sizeof(value) > shared->theMaxItemSize)
        throw ItemTooLarge();

    if (empty())
        throw Empty();

    const bool wasFull = full();
    const unsigned int pos =
        shared->theOut++ % shared->theCapacity * shared->theMaxItemSize;
    memcpy(&value, shared->theBuffer + pos, sizeof(value));
    --shared->theSize;
    return wasFull;
}

template <class Value>
bool
OneToOneUniQueue::push(const Value &value)
{
    if (sizeof(value) > shared->theMaxItemSize)
        throw ItemTooLarge();

    if (full())
        throw Full();

    const bool wasEmpty = empty();
    const unsigned int pos =
        shared->theIn++ % shared->theCapacity * shared->theMaxItemSize;
    memcpy(shared->theBuffer + pos, &value, sizeof(value));
    ++shared->theSize;
    return wasEmpty;
}


// FewToOneBiQueue

template <class Value>
bool
FewToOneBiQueue::pop(int &workerId, Value &value)
{
    ++theLastPopWorkerId;
    for (int i = 0; i < theWorkerCount; ++i) {
        theLastPopWorkerId = (theLastPopWorkerId + 1) % theWorkerCount;
        try {
            const bool wasFull = biQueues[theLastPopWorkerId]->pop(value);
            workerId = theLastPopWorkerId;
            return wasFull;
        } catch (const Empty &) {}
    }
    throw Empty();
}

template <class Value>
bool
FewToOneBiQueue::push(const int workerId, const Value &value)
{
    assert(validWorkerId(workerId));
    return biQueues[workerId]->push(value);
}

#endif // SQUID_IPC_QUEUE_H
