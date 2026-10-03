// Created by Chris Manlove

#include <csignal>
#include <cstdint>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include "test.hpp"

int32_t Test::resultPipe_ = -1;
const char* Test::statusNames_[static_cast<int32_t>(Status::Count)] = {
    "PASS", "FAIL", "CRASH", "TIMEOUT", "TODO", "ERROR", "BAD TEST"
};

const char* Test::statusColors_[static_cast<int32_t>(Status::Count)] = {
    GREEN, RED, BOLD_RED, YELLOW, CYAN, MAGENTA, BOLD_YELLOW
};

Test::Test(int32_t number, std::string name)
    : name_(name)
    , number_(number)
{}

void Test::RunAll() {
    std::cout << BOLD << "Challenge " << number_ << ": " << name_ << RESET << '\n';
    std::cout << "*Fastest of up to " << MAX_TIMED_RUNS << " runs\n";
    std::cout << "*Yellow = " << YELLOW_RATIO << "x slower, Red = " << RED_RATIO << "x slower (gaps under "
              << MIN_GAP_NANOSECONDS << " ns ignored)\n\n";
    std::cout << std::right << std::setw(NUMBER_COLUMN_WIDTH) << "#" << "  "
              << std::left << std::setw(RESULT_COLUMN_WIDTH) << "Result"
              << std::setw(TIME_COLUMN_WIDTH) << "You"
              << std::setw(TIME_COLUMN_WIDTH) << "Solution"
              << std::setw(TIME_COLUMN_WIDTH) << "STL"
              << "Description\n";

    RunTests();

    std::cout << '\n';
    for (int32_t i = 0; i < static_cast<int32_t>(Status::Count); ++i) {
        if (statusCounts_[i] > 0) {
            std::cout << statusColors_[i] << statusCounts_[i] << ' ' << statusNames_[i] << RESET << "  ";
        }
    }
    int32_t passed = statusCounts_[static_cast<int32_t>(Status::Pass)];
    std::cout << '\n' << BOLD << passed << '/' << caseCount_ << " passed" << RESET << std::endl;
}

void Test::Check(bool condition, const char* message, std::source_location location) {
    if (condition) { return; }

    const char* fileName = std::strrchr(location.file_name(), '/');
    fileName = (fileName == nullptr)? location.file_name() : fileName + 1;

    std::string text = std::string(message) + " (" + fileName + ":" + std::to_string(location.line()) + ")";
    WriteToPipe(text.data(), text.size());
    ExitChild(Status::Fail);
}

void Test::Todo() {
    ExitChild(Status::Todo);
}

void Test::Run(
    const std::string& description,
    uint32_t timeoutSeconds,
    TestFunction userTest,
    TestFunction solutionTest,
    TestFunction stlTest) {
    ++caseCount_;

    Outcome solution = RunInChild(solutionTest, timeoutSeconds);
    Outcome stl = (stlTest == nullptr)? Outcome() : RunInChild(stlTest, timeoutSeconds);
    Outcome user = RunInChild(userTest, timeoutSeconds);

    Status result = user.Result;
    std::string message = user.Message;
    if (solution.Result != Status::Pass) {
        result = Status::BadTest;
        message = BadTestMessage("solution", solution);
    } else if (stl.Result != Status::Pass) {
        result = Status::BadTest;
        message = BadTestMessage("stl", stl);
    }

    uint64_t fastest = std::numeric_limits<uint64_t>::max();
    if (user.Result == Status::Pass) { fastest = std::min(fastest, user.Nanoseconds); }
    if (solution.Result == Status::Pass) { fastest = std::min(fastest, solution.Nanoseconds); }
    if (stlTest != nullptr && stl.Result == Status::Pass) { fastest = std::min(fastest, stl.Nanoseconds); }

    std::string stlCell = (stlTest == nullptr)? PadCell("n/a") : TimeCell(stl, fastest, timeoutSeconds);

    int32_t index = static_cast<int32_t>(result);
    ++statusCounts_[index];

    std::cout << std::right << std::setw(NUMBER_COLUMN_WIDTH) << caseCount_ << "  "
              << statusColors_[index] << std::left << std::setw(RESULT_COLUMN_WIDTH) << statusNames_[index] << RESET
              << TimeCell(user, fastest, timeoutSeconds)
              << TimeCell(solution, fastest, timeoutSeconds)
              << stlCell
              << description << '\n';

    if (!message.empty()) {
        std::cout << "     " << statusColors_[index] << message << RESET << '\n';
    }
}

void Test::WriteToPipe(const void* data, size_t size) {
    const char* bytes = static_cast<const char*>(data);
    while (size > 0) {
        ssize_t written = write(resultPipe_, bytes, size);
        if (written <= 0) {
            return;
        }
        bytes += written;
        size -= static_cast<size_t>(written);
    }
}

void Test::ExitChild(Status status) {
    std::cout.flush();
    _exit(EXIT_CODE_OFFSET + static_cast<int32_t>(status));
}

Test::Outcome Test::RunInChild(TestFunction test, uint32_t timeoutSeconds) {
    int32_t fds[2];
    if (pipe(fds) != 0) {
        std::cout << "pipe() failed\n";
        std::exit(1);
    }

    std::cout.flush();
    pid_t pid = fork();

    if (pid == 0) {
        close(fds[0]);
        resultPipe_ = fds[1];
        alarm(timeoutSeconds);

        try {
            test();
            alarm(TIME_BUDGET_SECONDS + 2 * timeoutSeconds);

            // volatile so the compiler can't inline the empty call and measure 0 overhead.
            TestFunction volatile empty = &Test::Empty;
            uint64_t overhead = FastestTime(empty);
            uint64_t time = FastestTime(test);
            uint64_t best = (time > overhead)? time - overhead : 0;

            WriteToPipe(&best, sizeof(best));
            ExitChild(Status::Pass);
        } catch (const std::exception& e) {
            std::string text = std::string("threw an exception: ") + e.what();
            WriteToPipe(text.data(), text.size());
            ExitChild(Status::Error);
        } catch (...) {
            std::string text = "threw something that is not a std::exception";
            WriteToPipe(text.data(), text.size());
            ExitChild(Status::Error);
        }
    }

    close(fds[1]);

    std::string output;
    char buffer[256];
    ssize_t bytesRead = 0;
    while ((bytesRead = read(fds[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, static_cast<size_t>(bytesRead));
    }
    close(fds[0]);

    int32_t waitStatus = 0;
    waitpid(pid, &waitStatus, 0);

    Outcome outcome;
    if (WIFSIGNALED(waitStatus)) {
        int32_t signalNumber = WTERMSIG(waitStatus);
        if (signalNumber == SIGALRM) {
            outcome.Result = Status::Timeout;
            outcome.Message = "took longer than " + std::to_string(timeoutSeconds) + " s";
            return outcome;
        }
        outcome.Result = Status::Crash;
        outcome.Message = strsignal(signalNumber);
        return outcome;
    }

    int32_t exitCode = WEXITSTATUS(waitStatus);
    int32_t code = exitCode - EXIT_CODE_OFFSET;
    if (code < 0 || code >= static_cast<int32_t>(Status::Count)) {
        outcome.Result = Status::Crash;
        outcome.Message = "exited early with code " + std::to_string(exitCode);
        return outcome;
    }

    outcome.Result = static_cast<Status>(code);
    if (outcome.Result == Status::Pass) {
        std::memcpy(&outcome.Nanoseconds, output.data(), sizeof(outcome.Nanoseconds));
    } else {
        outcome.Message = output;
    }
    return outcome;
}

std::string Test::BadTestMessage(const char* who, const Outcome& outcome) {
    std::string message = std::string(who) + " got " + statusNames_[static_cast<int32_t>(outcome.Result)];
    if (!outcome.Message.empty()) {
        message += ": " + outcome.Message;
    }
    return message;
}

void Test::Empty() {}

uint64_t Test::FastestTime(TestFunction test) {
    uint64_t best = std::numeric_limits<uint64_t>::max();
    uint64_t total = 0;
    for (int32_t run = 0; run < MAX_TIMED_RUNS && total < TIME_BUDGET_NANOSECONDS; ++run) {
        auto start = std::chrono::steady_clock::now();
        test();
        auto end = std::chrono::steady_clock::now();

        std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        uint64_t elapsed = static_cast<uint64_t>(duration.count());
        total += elapsed;
        if (elapsed < best) {
            best = elapsed;
        }
    }
    return best;
}

std::string Test::FormatTime(uint64_t nanoseconds) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1);
    if (nanoseconds < 1'000) {
        out << nanoseconds << " ns";
    } else if (nanoseconds < 1'000'000) {
        out << nanoseconds / 1'000.0 << " us";
    } else if (nanoseconds < 1'000'000'000) {
        out << nanoseconds / 1'000'000.0 << " ms";
    } else {
        out << nanoseconds / 1'000'000'000.0 << " s";
    }
    return out.str();
}

std::string Test::PadCell(const std::string& text, const char* color) {
    std::string padding(TIME_COLUMN_WIDTH - text.size(), ' ');
    if (color == nullptr) {
        return text + padding;
    }
    return color + text + RESET + padding;
}

std::string Test::TimeCell(const Outcome& outcome, uint64_t fastest, uint32_t timeoutSeconds) {
    if (outcome.Result == Status::Timeout) {
        return PadCell(">" + std::to_string(timeoutSeconds) + " s");
    }
    if (outcome.Result != Status::Pass) {
        return PadCell("-");
    }

    double ratio = static_cast<double>(outcome.Nanoseconds) / static_cast<double>(std::max(fastest, uint64_t(1)));
    const char* color = GREEN;
    if (outcome.Nanoseconds - fastest >= MIN_GAP_NANOSECONDS) {
        if (ratio >= RED_RATIO) {
            color = RED;
        } else if (ratio >= YELLOW_RATIO) {
            color = YELLOW;
        }
    }
    return PadCell(FormatTime(outcome.Nanoseconds), color);
}
