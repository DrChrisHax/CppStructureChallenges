// Created by Chris Manlove

#include <csignal>
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

int Test::resultPipe_ = -1;
const char* Test::statusNames_[static_cast<int>(Status::Count)] = {
    "PASS", "FAIL", "CRASH", "TIMEOUT", "TODO", "ERROR", "BAD TEST"
};

const char* Test::statusColors_[static_cast<int>(Status::Count)] = {
    GREEN, RED, BOLD_RED, YELLOW, CYAN, MAGENTA, BOLD_YELLOW
};

Test::Test(int number, std::string name)
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
    for (int i = 0; i < static_cast<int>(Status::Count); ++i) {
        if (statusCounts_[i] > 0) {
            std::cout << statusColors_[i] << statusCounts_[i] << ' ' << statusNames_[i] << RESET << "  ";
        }
    }
    int passed = statusCounts_[static_cast<int>(Status::Pass)];
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
    unsigned timeoutSeconds,
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

    long long fastest = std::numeric_limits<long long>::max();
    if (user.Result == Status::Pass) { fastest = std::min(fastest, user.Nanoseconds); }
    if (solution.Result == Status::Pass) { fastest = std::min(fastest, solution.Nanoseconds); }
    if (stlTest != nullptr && stl.Result == Status::Pass) { fastest = std::min(fastest, stl.Nanoseconds); }

    std::string stlCell = (stlTest == nullptr)? PadCell("n/a") : TimeCell(stl, fastest, timeoutSeconds);

    int index = static_cast<int>(result);
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

void Test::WriteToPipe(const void* data, std::size_t size) {
    const char* bytes = static_cast<const char*>(data);
    while (size > 0) {
        ssize_t written = write(resultPipe_, bytes, size);
        if (written <= 0) {
            return;
        }
        bytes += written;
        size -= static_cast<std::size_t>(written);
    }
}

void Test::ExitChild(Status status) {
    std::cout.flush();
    _exit(EXIT_CODE_OFFSET + static_cast<int>(status));
}

Test::Outcome Test::RunInChild(TestFunction test, unsigned timeoutSeconds) {
    int fds[2];
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
            long long overhead = FastestTime(empty);
            long long best = std::max(FastestTime(test) - overhead, 0LL);

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
        output.append(buffer, static_cast<std::size_t>(bytesRead));
    }
    close(fds[0]);

    int waitStatus = 0;
    waitpid(pid, &waitStatus, 0);

    Outcome outcome;
    if (WIFSIGNALED(waitStatus)) {
        int signalNumber = WTERMSIG(waitStatus);
        if (signalNumber == SIGALRM) {
            outcome.Result = Status::Timeout;
            outcome.Message = "took longer than " + std::to_string(timeoutSeconds) + " s";
            return outcome;
        }
        outcome.Result = Status::Crash;
        outcome.Message = strsignal(signalNumber);
        return outcome;
    }

    int exitCode = WEXITSTATUS(waitStatus);
    int code = exitCode - EXIT_CODE_OFFSET;
    if (code < 0 || code >= static_cast<int>(Status::Count)) {
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
    std::string message = std::string(who) + " got " + statusNames_[static_cast<int>(outcome.Result)];
    if (!outcome.Message.empty()) {
        message += ": " + outcome.Message;
    }
    return message;
}

void Test::Empty() {}

long long Test::FastestTime(TestFunction test) {
    long long best = -1;
    long long total = 0;
    for (int run = 0; run < MAX_TIMED_RUNS && total < TIME_BUDGET_NANOSECONDS; ++run) {
        auto start = std::chrono::steady_clock::now();
        test();
        auto end = std::chrono::steady_clock::now();

        long long elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        total += elapsed;
        if (best < 0 || elapsed < best) {
            best = elapsed;
        }
    }
    return best;
}

std::string Test::FormatTime(long long nanoseconds) {
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

std::string Test::TimeCell(const Outcome& outcome, long long fastest, unsigned timeoutSeconds) {
    if (outcome.Result == Status::Timeout) {
        return PadCell(">" + std::to_string(timeoutSeconds) + " s");
    }
    if (outcome.Result != Status::Pass) {
        return PadCell("-");
    }

    double ratio = static_cast<double>(outcome.Nanoseconds) / static_cast<double>(std::max(fastest, 1LL));
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
