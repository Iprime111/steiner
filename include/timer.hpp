#pragma once

#include <chrono>
#include "log.hpp"

namespace steiner {
template <typename Duration, typename Clock = std::chrono::high_resolution_clock>
class Timer final {
    enum class TimerStatus {
        kIdle,
        kMeasuring,
        kNotInitialized,
    };
 
  public:
    class Lock {
        friend Timer;
        using TimeType = decltype(Clock::now());
        
        Lock(Timer& timer) : timer_(timer) {
            timer_.status_ = TimerStatus::kMeasuring;
            start_ = Clock::now();
        }
        
        Lock(const Lock&) = delete;
        Lock(Lock&&) = default;
        Lock& operator=(const Lock&) = delete;
        Lock& operator=(Lock&&) = default;
        
        TimeType start_;
        Timer& timer_;
        
      public:
        ~Lock() {
            auto now = Clock::now();
            timer_.duration_ = now - start_;
            timer_.status_ = TimerStatus::kIdle;
        }
    };
    
    Lock measure_scope() {
        if (status_ == TimerStatus::kMeasuring) {
            LOG_CRITICAL("Timer is already busy");
        }
        
        return {*this};
    }
    
    Duration result() const {
        if (status_ != TimerStatus::kIdle) {
            LOG_CRITICAL("Unable to return timer result");
        }
        
        return duration_;
    }
    
  private:
    TimerStatus status_{TimerStatus::kNotInitialized};
    Duration duration_{};
};
}  // namespace steiner
