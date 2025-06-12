#ifndef MREQ_LOCKGUARD_HPP
#define MREQ_LOCKGUARD_HPP

namespace mreq {

/**
 * @brief A RAII (Resource Acquisition Is Initialization) class template for automatic mutex locking and unlocking.
 *
 * This class ensures that a mutex is locked upon construction and automatically unlocked upon destruction,
 * providing exception safety and simplifying mutex management.
 *
 * @tparam MutexType The type of mutex to guard. It must provide `lock()` and `unlock()` methods.
 *                   Defaults to `Mutex` if not specified.
 */
template <typename MutexType = Mutex>
class LockGuard {
 public:
  /**
   * @brief Constructs a LockGuard and locks the provided mutex.
   * @param m The mutex instance to guard.
   */
  explicit LockGuard(MutexType &m) : mutex_(m) { mutex_.lock(); }

  /**
   * @brief Destroys the LockGuard and unlocks the associated mutex.
   */
  ~LockGuard() { mutex_.unlock(); }

  // Delete copy constructor and assignment operator to prevent accidental copying.
  LockGuard(const LockGuard &) = delete;
  LockGuard &operator=(const LockGuard &) = delete;

 private:
  MutexType &mutex_; ///< Reference to the mutex being guarded.
};

} // namespace mreq

#endif // MREQ_LOCKGUARD_HPP 