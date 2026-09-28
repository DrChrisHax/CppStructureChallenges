// Created by Chris Manlove

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
        const char* message = "check failed",
        std::source_location location = std::source_location::current());
    [[noreturn]] static void Todo();

protected:
    virtual void RunTests() = 0;

    void Run(
        const std::string& description,
        unsigned timeoutSeconds,
        TestFunction userTest,
        TestFunction solutionTest,
        TestFunction stlTest = nullptr);

private:
    enum class Status { Pass, Fail, Crash, Timeout, Todo, Error, BadTest, Count };

    struct Outcome {
        Status Result = Status::Pass;
        long long Nanoseconds = 0;
        std::string Message;
    };

    static constexpr int MAX_TIMED_RUNS = 1000;
    static constexpr unsigned TIME_BUDGET_SECONDS = 1;
    static constexpr long long TIME_BUDGET_NANOSECONDS = TIME_BUDGET_SECONDS * 1'000'000'000LL;
    static constexpr int EXIT_CODE_OFFSET = 100;
    static constexpr int NUMBER_COLUMN_WIDTH = 3;
    static constexpr int RESULT_COLUMN_WIDTH = 10;
    static constexpr std::size_t TIME_COLUMN_WIDTH = 12;
    static constexpr long long MIN_GAP_NANOSECONDS = 50;
    static constexpr double YELLOW_RATIO = 2.0;
    static constexpr double RED_RATIO = 10.0;

    [[noreturn]] static void ExitChild(Status status);

    static void WriteToPipe(const void* data, std::size_t size);
    static void Empty();
    static long long FastestTime(TestFunction test);
    static Outcome RunInChild(TestFunction test, unsigned timeoutSeconds);
    static std::string BadTestMessage(const char* who, const Outcome& outcome);
    static std::string FormatTime(long long nanoseconds);
    static std::string PadCell(const std::string& text, const char* color = nullptr);
    static std::string TimeCell(const Outcome& outcome, long long fastest, unsigned timeoutSeconds);

    static int resultPipe_;
    static const char* statusNames_[static_cast<int>(Status::Count)];
    static const char* statusColors_[static_cast<int>(Status::Count)];

    std::string name_;
    int number_;
    int caseCount_ = 0;
    int statusCounts_[static_cast<int>(Status::Count)] = {};
};
