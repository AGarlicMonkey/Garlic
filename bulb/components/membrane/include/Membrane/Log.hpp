#pragma once

namespace garlic::membrane {
    public enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical,
    };
}

namespace garlic::membrane {
    public delegate void LogSink(System::String ^ message);

    /**
     * @brief Managed Logger wrapper design to be called from Bulb.
     */
    public ref class Log{
    public:
        static void write(LogLevel level, System::String ^ message);

        static void addSink(LogSink ^ sink, System::String ^ pattern);
    };
}