#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <Clove/Definitions.hpp>

#define CLOVE_DECLARE_LOG_CATEGORY(categoryName)                 \
    struct LOG_CATEGORY_##categoryName {                         \
        static std::string_view constexpr name{ #categoryName }; \
    };

namespace spdlog {
    class logger;
    namespace sinks {
        class sink;
    }
}

namespace garlic::clove {
    enum class LogLevel {
        Trace,    /**< Extra info which is usually uneccessary. */
        Debug,    /**< Anything which could help with debugging. */
        Info,     /**< Useful information to present to the user. */
        Warning,  /**< When something happened that might cause an error. */
        Error,    /**< Something has gone wrong but is recoverable. */
        Critical, /**< Someithng has gone wrong and the application needs to exit. */
    };
}

namespace garlic::clove {
    class Logger {
        //VARIABLES
    private:
        std::unique_ptr<spdlog::logger> logger;

        //FUNCTIONS
    public:
        Logger();
        ~Logger();

        static inline Logger &get();

        template<typename... Args>
        void log(std::string_view category, LogLevel level, std::string_view msg, Args &&... args);

        void addSink(std::shared_ptr<spdlog::sinks::sink> sink);

    private:
        void doLog(LogLevel level, std::string_view msg);
    };
}

#define CLOVE_LOG(category, level, ...) ::garlic::clove::Logger::get().log(category::name, level, __VA_ARGS__);

#if CLOVE_ENABLE_ASSERTIONS
    #define CLOVE_ASSERT(x, ...)                                                                                          \
        {                                                                                                                 \
            if(!(x)) {                                                                                                    \
                CLOVE_LOG(LOG_CATEGORY_CLOVE, ::garlic::clove::LogLevel::Critical, "Assertion Failed: {0}", __VA_ARGS__); \
                CLOVE_DEBUG_BREAK;                                                                                        \
            }                                                                                                             \
        }
#else
    #define CLOVE_ASSERT(x, ...) (x)
#endif

#include "Log.inl"

CLOVE_DECLARE_LOG_CATEGORY(CLOVE)