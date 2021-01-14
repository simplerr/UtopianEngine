#pragma once

#include <memory>

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

/* Time conversion constants */
#define HOURS_PER_DAY         (24.0f)
#define MINUTES_PER_HOUR      (60.0f)
#define SECONDS_PER_HOUR      ((SECONDS_PER_MINUTE) * (MINUTES_PER_HOUR))
#define SECONDS_PER_MINUTE    (60.0f)
#define MS_PER_S              (1000.0f)
#define US_PER_S              ((US_PER_MS) * (MS_PER_S))
#define US_PER_MS             (1000.0f)
#define NS_PER_S              ((NS_PER_MS) * (MS_PER_S))
#define NS_PER_MS             ((NS_PER_US) * (US_PER_MS))
#define NS_PER_US             (1000.0f)
