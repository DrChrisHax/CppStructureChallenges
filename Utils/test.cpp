#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
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
    std::cout << BOLD << "Challenge " << number_ << ": " << name_ << RESET << "\n\n";
    std::cout << std::right << std::setw(3) << "#" << "  " << std::left << std::setw(10) << "Result"
              << std::setw(12) << "You" << std::setw(12) << "Solution" << "Description\n";

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

void Test::Check(bool condition, const std::string& message, std::source_location location) {
    if (condition) { return; }

    const char* fileName = std::strrchr(location.file_name(), '/');
    fileName = (fileName == nullptr)? location.file_name() : fileName + 1;

    std::string text = message + " (" + fileName + ":" + std::to_string(location.line()) + ")";
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
    TestFunction solutionTest) {
    ++caseCount_;

    long long solutionTime = 0;
    std::string solutionMessage;
    Status solutionStatus = RunInChild(solutionTest, timeoutSeconds, solutionTime, solutionMessage);

    long long userTime = 0;
    std::string userMessage;
    Status userStatus = RunInChild(userTest, timeoutSeconds, userTime, userMessage);

    Status result = userStatus;
    std::string message = userMessage;

    if (solutionStatus != Status::Pass) {
        result = Status::BadTest;
        message = "solution got " + std::string(statusNames_[static_cast<int>(solutionStatus)]);
        if (!solutionMessage.empty()) {
            message += ": " + solutionMessage;
        }
    }

    int index = static_cast<int>(result);
    ++statusCounts_[index];

    std::cout << std::right << std::setw(3) << caseCount_ << "  "
              << statusColors_[index] << std::left << std::setw(10) << statusNames_[index] << RESET
              << std::setw(12) << TimeCell(userStatus, userTime, timeoutSeconds)
              << std::setw(12) << TimeCell(solutionStatus, solutionTime, timeoutSeconds)
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
    _exit(100 + static_cast<int>(status));
}

Test::Status Test::RunInChild(
    TestFunction test,
    unsigned timeoutSeconds,
    long long& nanoseconds,
    std::string& message) {
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
            auto start = std::chrono::steady_clock::now();
            test();
            auto end = std::chrono::steady_clock::now();

            long long elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            WriteToPipe(&elapsed, sizeof(elapsed));
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

    if (WIFSIGNALED(waitStatus)) {
        int signalNumber = WTERMSIG(waitStatus);
        if (signalNumber == SIGALRM) {
            message = "took longer than " + std::to_string(timeoutSeconds) + " s";
            return Status::Timeout;
        }
        message = strsignal(signalNumber);
        return Status::Crash;
    }

    int exitCode = WEXITSTATUS(waitStatus);
    int code = exitCode - 100;
    if (code < 0 || code >= static_cast<int>(Status::Count)) {
        message = "exited early with code " + std::to_string(exitCode);
        return Status::Crash;
    }

    Status status = static_cast<Status>(code);
    if (status == Status::Pass) {
        std::memcpy(&nanoseconds, output.data(), sizeof(nanoseconds));
    } else {
        message = output;
    }
    return status;
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

std::string Test::TimeCell(Status status, long long nanoseconds, unsigned timeoutSeconds) {
    if (status == Status::Pass) {
        return FormatTime(nanoseconds);
    }
    if (status == Status::Timeout) {
        return ">" + std::to_string(timeoutSeconds) + " s";
    }
    return "-";
}
