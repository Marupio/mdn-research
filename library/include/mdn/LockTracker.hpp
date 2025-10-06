#pragma once
#include <atomic>
#include <chrono>
#include <shared_mutex>
#include <thread>
#include <utility>

#ifdef MDN_DEBUG
  #include <unordered_map>
  #include <cstdint>
  #include <sstream>
#endif

#include <mdn/Logger.hpp>

namespace mdn {

#ifdef MDN_DEBUG
struct _LTThreadCounts {
    std::unordered_map<const void*, int> rd, wr;
};
inline thread_local _LTThreadCounts g_lt_counts;
#endif

class LockTracker {
public:
    class WritableLock {
    public:
        explicit WritableLock(std::shared_mutex& m, std::atomic<int>& ctr) :
            lk_(m, std::defer_lock), ctr_(&ctr)
        {
#ifdef MDN_DEBUG
            const void* key = static_cast<const void*>(&m);
            const int rdHeld = g_lt_counts.rd[key];
            const int wrHeld = g_lt_counts.wr[key];
            if (rdHeld > 0 || wrHeld > 0) {
                Log_Warn(
                    "Potential self-deadlock: acquiring UNIQUE while already holding "
                        << (wrHeld ? "UNIQUE" : "SHARED") << " on same mutex."
                );
            }

            // Try-lock loop with a soft timeout to emit a warning before blocking indefinitely
            using clock = std::chrono::steady_clock;
            const auto start = clock::now();
            const auto warn_after = std::chrono::milliseconds(50);
            while (!lk_.try_lock()) {
                if (clock::now() - start >= warn_after) {
                    Log_Warn("Lock wait >50ms (unique). Possible deadlock.");
                    break;
                }
                std::this_thread::yield();
            }
            if (!lk_.owns_lock()) {
                lk_.lock(); // block finally
            }
            g_lt_counts.wr[key] += 1;
#else
            lk_.lock();
#endif
            engaged_ = true;
            ctr_->fetch_add(1, std::memory_order_relaxed);
        }

        WritableLock(WritableLock&& o) noexcept :
            lk_(std::move(o.lk_)),
            ctr_(std::exchange(o.ctr_, nullptr)),
            engaged_(std::exchange(o.engaged_, false))
        {}

        WritableLock& operator=(WritableLock&& o) noexcept {
            if (this != &o) {
                release();
                lk_      = std::move(o.lk_);
                ctr_     = std::exchange(o.ctr_, nullptr);
                engaged_ = std::exchange(o.engaged_, false);
            }
            return *this;
        }

        ~WritableLock() {
            release();
        }

        bool owns_lock() const noexcept {
            return lk_.owns_lock();
        }
        explicit operator bool() const noexcept {
            return owns_lock();
        }

        void unlock() {
            if (engaged_) {
#ifdef MDN_DEBUG
                Log_Debug4("");
                const void* key = static_cast<const void*>(lk_.mutex());
                if (key) { // mutex() may be null if moved-from
                    auto it = g_lt_counts.wr.find(key);
                    if (it != g_lt_counts.wr.end() && it->second > 0) --(it->second);
                }
#endif
                lk_.unlock();
                ctr_->fetch_sub(1, std::memory_order_relaxed);
                engaged_ = false;
            }
        }

        // Optional pass-throughs, possibly for later:
        // void lock() { lk_.lock(); } // (remember to increment on successful lock)
        // std::shared_mutex* mutex() const noexcept { return lk_.mutex(); }

    private:
        void release() {
            if (engaged_) {
#ifdef MDN_DEBUG
                Log_Debug4("");
                const void* key = static_cast<const void*>(lk_.mutex());
                if (key) {
                    auto it = g_lt_counts.wr.find(key);
                    if (it != g_lt_counts.wr.end() && it->second > 0) --(it->second);
                }
#endif
                lk_.unlock();

                ctr_->fetch_sub(1, std::memory_order_relaxed);
                engaged_ = false;
            }
        }

        std::unique_lock<std::shared_mutex> lk_;
        std::atomic<int>* ctr_{nullptr};
        bool engaged_{false};
    };

    class ReadOnlyLock {
    public:
        explicit ReadOnlyLock(std::shared_mutex& m, std::atomic<int>& ctr)
            : lk_(m, std::defer_lock), ctr_(&ctr)
        {
#ifdef MDN_DEBUG
            const void* key = static_cast<const void*>(&m);
            // Soft timeout warning for shared acquisition as well
            using clock = std::chrono::steady_clock;
            const auto start = clock::now();
            const auto warn_after = std::chrono::milliseconds(50);
            while (!lk_.try_lock()) {
                if (clock::now() - start >= warn_after) {
                    Log_Warn("Lock wait >50ms (shared). Possible contention/deadlock.");
                    break;
                }
                std::this_thread::yield();
            }
            if (!lk_.owns_lock()) {
                lk_.lock(); // block
            }
            g_lt_counts.rd[key] += 1;
#else
            lk_.lock();
#endif
            engaged_ = true;
            ctr_->fetch_add(1, std::memory_order_relaxed);
        }

        ReadOnlyLock(ReadOnlyLock&& o) noexcept :
            lk_(std::move(o.lk_)),
            ctr_(std::exchange(o.ctr_, nullptr)),
            engaged_(std::exchange(o.engaged_, false))
        {}

        ReadOnlyLock& operator=(ReadOnlyLock&& o) noexcept {
            if (this != &o) {
                release();
                lk_      = std::move(o.lk_);
                ctr_     = std::exchange(o.ctr_, nullptr);
                engaged_ = std::exchange(o.engaged_, false);
            }
            return *this;
        }

        ~ReadOnlyLock() {
            release();
        }

        bool owns_lock() const noexcept {
            return lk_.owns_lock();
        }
        explicit operator bool() const noexcept {
            return owns_lock();
        }

        void unlock() {
            if (engaged_) {
#ifdef MDN_DEBUG
                Log_Debug4("");
                const void* key = static_cast<const void*>(lk_.mutex());
                if (key) {
                    auto it = g_lt_counts.rd.find(key);
                    if (it != g_lt_counts.rd.end() && it->second > 0) --(it->second);
                }
#endif
                lk_.unlock();
                ctr_->fetch_sub(1, std::memory_order_relaxed);
                engaged_ = false;
            }
        }

    private:
        void release() {
            if (engaged_) {
#ifdef MDN_DEBUG
                Log_Debug4("");
                const void* key = static_cast<const void*>(lk_.mutex());
                if (key) {
                    auto it = g_lt_counts.rd.find(key);
                    if (it != g_lt_counts.rd.end() && it->second > 0) --(it->second);
                }
#endif
                lk_.unlock();
                ctr_->fetch_sub(1, std::memory_order_relaxed);
                engaged_ = false;
            }
        }

        std::shared_lock<std::shared_mutex> lk_;
        std::atomic<int>* ctr_{nullptr};
        bool engaged_{false};
    };

    // Factory helpers (const so you can call them from const methods)
    WritableLock lockWriteable(std::shared_mutex& m) const {
        return WritableLock(m, writers_);
    }
    ReadOnlyLock lockReadOnly(std::shared_mutex& m) const {
        return ReadOnlyLock(m, readers_);
    }

    int activeWriterCount() const noexcept {
        return writers_.load(std::memory_order_relaxed);
    }
    int activeReaderCount() const noexcept {
        return readers_.load(std::memory_order_relaxed);
    }

private:
    mutable std::atomic<int> writers_{0};
    mutable std::atomic<int> readers_{0};
};

} // namespace mdn
