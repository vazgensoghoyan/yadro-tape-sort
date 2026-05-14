#include <gtest/gtest.h>

#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <chrono>

#include "app/app.hpp"

using namespace tape_sort::app;

namespace fs = std::filesystem;

namespace {

class AppTest : public ::testing::Test {
protected:
    struct TestFiles {
        std::string input;
        std::string output;
        std::string config;
    };

    void SetUp() override {
        base_dir = "tmp_test_" + unique_suffix();
        fs::create_directories(base_dir);
        fs::create_directories(base_dir + "/tmp");
    }

    void TearDown() override {
        std::error_code ec;
        fs::remove_all(base_dir, ec);
    }

    TestFiles make_files() {
        return TestFiles{
            .input  = base_dir + "/input.bin",
            .output = base_dir + "/output.bin",
            .config = base_dir + "/config.cfg"
        };
    }

    void write_binary(const std::string& path, const std::vector<int32_t>& data) {
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        for (auto v : data)
            f.write(reinterpret_cast<const char*>(&v), sizeof(v));
    }

    void write_config_full(const std::string& path, size_t memory_limit = 1024) {
        std::ofstream f(path);
        f <<
            "tape.read_delay_ms=0\n"
            "tape.write_delay_ms=0\n"
            "tape.move_delay_ms=0\n"
            "tape.rewind_delay_ms=0\n"
            "memory.limit_bytes=" << memory_limit << "\n"
            "filesystem.tmp_dir=" << base_dir << "/tmp\n"
            "log.enabled=false\n";
    }

    std::vector<int32_t> read_binary(const std::string& path) {
        std::ifstream f(path, std::ios::binary);

        std::vector<int32_t> result;
        int32_t x;

        while (f.read(reinterpret_cast<char*>(&x), sizeof(x)))
            result.push_back(x);

        return result;
    }

    ExitCode run_app(const TestFiles& files) {
        std::vector<std::string> args = {
            "app",
            files.input,
            files.output,
            files.config
        };

        std::vector<char*> argv;
        for (auto& s : args)
            argv.push_back(s.data());

        App app;
        return app.run(static_cast<int>(argv.size()), argv.data());
    }

    std::string unique_suffix() {
        return std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()
        );
    }

protected:
    std::string base_dir;
};

} // namespace

TEST_F(AppTest, SortsUnsortedArray) {
    auto files = make_files();

    write_binary(files.input, {5, 1, 4, 2, 3});
    write_config_full(files.config);

    EXPECT_EQ(run_app(files), ExitCode::Success);

    EXPECT_EQ(read_binary(files.output),
              (std::vector<int32_t>{1, 2, 3, 4, 5}));
}

TEST_F(AppTest, AlreadySortedArray) {
    auto files = make_files();

    write_binary(files.input, {1, 2, 3, 4, 5});
    write_config_full(files.config);

    EXPECT_EQ(run_app(files), ExitCode::Success);

    EXPECT_EQ(read_binary(files.output),
              (std::vector<int32_t>{1, 2, 3, 4, 5}));
}

TEST_F(AppTest, ReverseSortedArray) {
    auto files = make_files();

    write_binary(files.input, {5, 4, 3, 2, 1});
    write_config_full(files.config);

    EXPECT_EQ(run_app(files), ExitCode::Success);

    EXPECT_EQ(read_binary(files.output),
              (std::vector<int32_t>{1, 2, 3, 4, 5}));
}

TEST_F(AppTest, SingleElementArray) {
    auto files = make_files();

    write_binary(files.input, {42});
    write_config_full(files.config);

    EXPECT_EQ(run_app(files), ExitCode::Success);

    EXPECT_EQ(read_binary(files.output),
              (std::vector<int32_t>{42}));
}

TEST_F(AppTest, EmptyInputFile) {
    auto files = make_files();

    write_binary(files.input, {});
    write_config_full(files.config);

    EXPECT_EQ(run_app(files), ExitCode::Success);
    EXPECT_TRUE(read_binary(files.output).empty());
}

TEST_F(AppTest, HandlesDuplicates) {
    auto files = make_files();

    write_binary(files.input, {5, 1, 5, 1, 5});
    write_config_full(files.config);

    EXPECT_EQ(run_app(files), ExitCode::Success);

    EXPECT_EQ(read_binary(files.output),
              (std::vector<int32_t>{1, 1, 5, 5, 5}));
}

TEST_F(AppTest, HandlesNegativeNumbers) {
    auto files = make_files();

    write_binary(files.input, {-1, -10, 5, 0, 3});
    write_config_full(files.config);

    EXPECT_EQ(run_app(files), ExitCode::Success);

    EXPECT_EQ(read_binary(files.output),
              (std::vector<int32_t>{-10, -1, 0, 3, 5}));
}

TEST_F(AppTest, RandomArrayCorrectness) {
    auto files = make_files();

    std::vector<int32_t> data(2000);

    for (auto& x : data)
        x = rand() % 10000 - 5000;

    write_binary(files.input, data);
    write_config_full(files.config);

    EXPECT_EQ(run_app(files), ExitCode::Success);

    auto expected = data;
    std::sort(expected.begin(), expected.end());

    EXPECT_EQ(read_binary(files.output), expected);
}

TEST_F(AppTest, LargeInputStressTest) {
    auto files = make_files();

    const int SIZE = 50'000;

    std::vector<int32_t> data;
    data.reserve(SIZE);

    for (int i = 0; i < SIZE; ++i)
        data.push_back(rand());

    write_binary(files.input, data);
    write_config_full(files.config);

    EXPECT_EQ(run_app(files), ExitCode::Success);

    auto expected = data;
    std::sort(expected.begin(), expected.end());

    EXPECT_EQ(read_binary(files.output), expected);
}

TEST_F(AppTest, VerySmallMemoryLimit) {
    auto files = make_files();

    write_binary(files.input, {5, 3, 1, 4, 2});
    write_config_full(files.config, 4); // 1 int32 only

    EXPECT_EQ(run_app(files), ExitCode::Success);

    EXPECT_EQ(read_binary(files.output),
              (std::vector<int32_t>{1, 2, 3, 4, 5}));
}

TEST_F(AppTest, InvalidArgumentsReturnError) {
    std::vector<std::string> args = {"app", "only_one_arg"};

    std::vector<char*> argv;
    for (auto& s : args)
        argv.push_back(s.data());

    App app;
    auto code = app.run(static_cast<int>(argv.size()), argv.data());

    EXPECT_NE(code, ExitCode::Success);
}
