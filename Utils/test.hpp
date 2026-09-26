#pragma once

#include <cstddef>

#include <source_location>
#include <string>

constexpr const char* RESET = "\033[0m";
constexpr const char* BOLD = "\033[1m";
constexpr const char* RED = "\033[31m";
constexpr const char* GREEN = "\033[32m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* MAGENTA = "\033[35m";
constexpr const char* CYAN = "\033[36m";
constexpr const char* BOLD_RED = "\033[1;31m";
constexpr const char* BOLD_YELLOW = "\033[1;33m";

class Test {
public:
    using TestFunction = void (*)();

    Test(int number, std::string name);
    virtual ~Test() = default;

    void RunAll();

    static void Check(
        bool condition,
        const std::string& message = "check failed",
        std::source_location location = std::source_location::current());
    [[noreturn]] static void Todo();

protected:
    virtual void RunTests() = 0;

    void Run(const std::string& description, unsigned timeoutSeconds, TestFunction userTest, TestFunction solutionTest);

private:
    enum class Status { Pass, Fail, Crash, Timeout, Todo, Error, BadTest, Count };

    [[noreturn]] static void ExitChild(Status status);

    static void WriteToPipe(const void* data, std::size_t size);
    static Status RunInChild(TestFunction test, unsigned timeoutSeconds, long long& nanoseconds, std::string& message);
    static std::string FormatTime(long long nanoseconds);
    static std::string TimeCell(Status status, long long nanoseconds, unsigned timeoutSeconds);

    static int resultPipe_;
    static const char* statusNames_[static_cast<int>(Status::Count)];
    static const char* statusColors_[static_cast<int>(Status::Count)];

    std::string name_;
    int number_;
    int caseCount_ = 0;
    int statusCounts_[static_cast<int>(Status::Count)] = {};
};
