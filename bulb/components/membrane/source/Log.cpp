#include "Membrane/Log.hpp"

#include <Clove/Log/Log.hpp>
#include <msclr/gcroot.h>
#include <msclr/marshal_cppstd.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/pattern_formatter.h>

//Making the assumption that this library will only be used with Bulb
CLOVE_DECLARE_LOG_CATEGORY(BULB);

namespace garlic::membrane {
    namespace {
        clove::LogLevel convertLevel(LogLevel level) {
            switch(level) {
                default:
                case LogLevel::Trace:
                    return clove::LogLevel::Trace;
                case LogLevel::Debug:
                    return clove::LogLevel::Debug;
                case LogLevel::Info:
                    return clove::LogLevel::Info;
                case LogLevel::Warning:
                    return clove::LogLevel::Warning;
                case LogLevel::Error:
                    return clove::LogLevel::Error;
                case LogLevel::Critical:
                    return clove::LogLevel::Critical;
            }
        }

        class DelegateSink : public spdlog::sinks::sink {
            //VARIABLES
        public:
            msclr::gcroot<LogSink ^> sink;
            std::unique_ptr<spdlog::formatter> formatter;

            //FUNCTIONS
        public:
            DelegateSink(msclr::gcroot<LogSink ^> sink)
                : sink(std::move(sink)) {
            }

            void log(spdlog::details::log_msg const &msg) override {
                spdlog::memory_buf_t formatted;
                formatter->format(msg, formatted);

                sink->Invoke(gcnew System::String(std::string{ formatted.data(), formatted.size() }.c_str()));
            }

            void flush() override {}

            void set_pattern(std::string const &pattern) override {
                set_formatter(spdlog::details::make_unique<spdlog::pattern_formatter>(pattern));
            }

            void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override {
                formatter = std::move(sink_formatter);
            }
        };
    }

    void Log::write(LogLevel level, System::String ^ message) {
        CLOVE_LOG(LOG_CATEGORY_BULB, convertLevel(level), msclr::interop::marshal_as<std::string>(message));
    }

    void Log::addSink(LogSink ^ sink, System::String ^ pattern) {
        auto delegateSink = std::make_shared<DelegateSink>(sink);
        delegateSink->set_pattern(msclr::interop::marshal_as<std::string>(pattern));

        delegateSink->set_level(spdlog::level::debug);

        clove::Logger::get().addSink(std::move(delegateSink));
    }
}