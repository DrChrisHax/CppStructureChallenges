// Created by Chris Manlove

#pragma once

#include <cstddef>
#include <cstdint>
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

    Test(int32_t number, std::string name);
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
        uint32_t timeoutSeconds,
        TestFunction userTest,
        TestFunction solutionTest,
        TestFunction stlTest = nullptr);

private:
    enum class Status { Pass, Fail, Crash, Timeout, Todo, Error, BadTest, Count };

    struct Outcome {
        Status Result = Status::Pass;
        uint64_t Nanoseconds = 0;
        std::string Message;
    };

    static constexpr int32_t MAX_TIMED_RUNS = 1000;
    static constexpr uint32_t TIME_BUDGET_SECONDS = 1;
    static constexpr uint64_t TIME_BUDGET_NANOSECONDS = TIME_BUDGET_SECONDS * 1'000'000'000ULL;
    static constexpr int32_t EXIT_CODE_OFFSET = 100;
    static constexpr int32_t NUMBER_COLUMN_WIDTH = 3;
    static constexpr int32_t RESULT_COLUMN_WIDTH = 10;
    static constexpr size_t TIME_COLUMN_WIDTH = 12;
    static constexpr uint64_t MIN_GAP_NANOSECONDS = 50;
    static constexpr double YELLOW_RATIO = 2.0;
    static constexpr double RED_RATIO = 10.0;

    [[noreturn]] static void ExitChild(Status status);

    static void WriteToPipe(const void* data, size_t size);
    static void Empty();
    static uint64_t FastestTime(TestFunction test);
    static Outcome RunInChild(TestFunction test, uint32_t timeoutSeconds);
    static std::string BadTestMessage(const char* who, const Outcome& outcome);
    static std::string FormatTime(uint64_t nanoseconds);
    static std::string PadCell(const std::string& text, const char* color = nullptr);
    static std::string TimeCell(const Outcome& outcome, uint64_t fastest, uint32_t timeoutSeconds);

    static int32_t resultPipe_;
    static const char* statusNames_[static_cast<int32_t>(Status::Count)];
    static const char* statusColors_[static_cast<int32_t>(Status::Count)];

    std::string name_;
    int32_t number_;
    int32_t caseCount_ = 0;
    int32_t statusCounts_[static_cast<int32_t>(Status::Count)] = {};
};
