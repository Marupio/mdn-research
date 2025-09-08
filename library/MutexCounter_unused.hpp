#pragma once

#include <memory>
#include <shared_mutex>

class MutexCounter {
    public:

        using WritableLock = std::unique_lock<std::shared_mutex>;
        using ReadOnlyLock = std::shared_lock<std::shared_mutex>;

        // Lock m_mutex for writeable reasons (unique_lock)
        WritableLock lockWriteable() const;
        ReadOnlyLock lockReadOnly() const;

        inline WritableLock takeWrite() {
            ++m_writeCount;
            return std::unique_lock(m_mutex);
        }
        inline ReadOnlyLock takeRead() {
            ++m_readCount;
            return std::shared_lock(m_mutex);
        }

    private:
        int m_writeCount;
        int m_readCount;
        mutable std::shared_mutex m_mutex;
};

class UniqueLock {
    std::unique_lock<std::shared_mutex> m_mutex;
    MutexCounter& m_parent;

    ~UniqueLock() {
        m_parent.giveBack(*this);
    }
};
