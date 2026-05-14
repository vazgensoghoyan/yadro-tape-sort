#include <gtest/gtest.h>

#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

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

protected:
    void SetUp() override {
        fs::create_directories("tmp_test");
    }

    void TearDown() override {
        fs::remove_all("tmp_test");
    }

    TestFiles make_files() {
        static int id = 0;

        return TestFiles{
            .input  = "tmp_test/input_" + std::to_string(id) + ".bin",
            .output = "tmp_test/output_" + std::to_string(id) + ".bin",
            .config = "tmp_test/config_" + std::to_string(id++) + ".cfg"
        };
    }

    void write_binary(const std::string& path, const std::vector<int32_t>& data) {
        std::ofstream f(path, std::ios::binary | std::ios::trunc);

        for (auto v : data)
            f.write(reinterpret_cast<const char*>(&v), sizeof(v));
    }

    void write_config(const std::string& path) {
        std::ofstream f(path);

        f <<
            "tape.read_delay_ms=0\n"
            "tape.write_delay_ms=0\n"
            "tape.move_delay_ms=0\n"
            "tape.rewind_delay_ms=0\n"
            "memory.limit_bytes=1024\n"
            "filesystem.tmp_dir=tmp_test/tmp\n"
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
        const char* argv[] = {
            "app",
            files.input.c_str(),
            files.output.c_str(),
            files.config.c_str()
        };

        App app;
        return app.run(4, const_cast<char**>(argv));
    }
};

} // namespace

// -------------------- TESTS --------------------

TEST_F(AppTest, SortsUnsortedArray)
{
    auto files = make_files();

    write_binary(files.input, {5, 1, 4, 2, 3});
    write_config(files.config);

    auto code = run_app(files);

    EXPECT_EQ(code, ExitCode::Success);
    EXPECT_EQ(read_binary(files.output), (std::vector<int32_t>{1, 2, 3, 4, 5}));
}

TEST_F(AppTest, AlreadySortedArray)
{
    auto files = make_files();

    write_binary(files.input, {1, 2, 3, 4, 5});
    write_config(files.config);

    auto code = run_app(files);

    EXPECT_EQ(code, ExitCode::Success);
    EXPECT_EQ(read_binary(files.output), (std::vector<int32_t>{1, 2, 3, 4, 5}));
}

TEST_F(AppTest, ReverseSortedArray)
{
    auto files = make_files();

    write_binary(files.input, {5, 4, 3, 2, 1});
    write_config(files.config);

    auto code = run_app(files);

    EXPECT_EQ(code, ExitCode::Success);
    EXPECT_EQ(read_binary(files.output), (std::vector<int32_t>{1, 2, 3, 4, 5}));
}

TEST_F(AppTest, SingleElementArray)
{
    auto files = make_files();

    write_binary(files.input, {42});
    write_config(files.config);

    auto code = run_app(files);

    EXPECT_EQ(code, ExitCode::Success);
    EXPECT_EQ(read_binary(files.output), (std::vector<int32_t>{42}));
}

TEST_F(AppTest, InvalidArgumentsReturnError)
{
    char* argv[] = {
        (char*)"app",
        (char*)"only_one_arg"
    };

    App app;
    auto code = app.run(2, argv);

    EXPECT_NE(code, ExitCode::Success);
}
