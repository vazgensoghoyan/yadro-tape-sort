#include <gtest/gtest.h>

#include <fstream>
#include <filesystem>

#include "config/config_loader.hpp"

using namespace tape_sort::config;

namespace fs = std::filesystem;

class ConfigLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        dir = fs::temp_directory_path() / "config_tests";
        fs::create_directories(dir);

        path = dir / "config.txt";
    }

    void TearDown() override {
        std::error_code ec;
        fs::remove_all(dir, ec);
    }

    void write_config(const std::string& content) {
        std::ofstream out(path);
        out << content;
    }

protected:
    fs::path dir;
    fs::path path;
};

TEST_F(ConfigLoaderTest, LoadsValidConfig)
{
    write_config(R"(
        tape.read_delay_ms = 1
        tape.write_delay_ms = 2
        tape.move_delay_ms = 3
        tape.rewind_delay_ms = 4

        memory.limit_bytes = 1024

        filesystem.tmp_dir = ./tmp
        log.enabled = true
    )");

    auto config = ConfigLoader::load(path.string());

    EXPECT_EQ(config.tape.read_delay_ms, 1);
    EXPECT_EQ(config.tape.write_delay_ms, 2);
    EXPECT_EQ(config.tape.move_delay_ms, 3);
    EXPECT_EQ(config.tape.rewind_delay_ms, 4);

    EXPECT_EQ(config.memory.limit_bytes, 1024);

    EXPECT_EQ(config.filesystem.tmp_dir, "./tmp");
    EXPECT_TRUE(config.log.enabled);
}

TEST_F(ConfigLoaderTest, IgnoresComments)
{
    write_config(R"(
        // this is comment
        tape.read_delay_ms = 5 // inline comment
        tape.write_delay_ms = 6

        tape.move_delay_ms = 1
        tape.rewind_delay_ms = 1

        memory.limit_bytes = 1024
        filesystem.tmp_dir = ./tmp
        log.enabled = true
    )");

    auto config = ConfigLoader::load(path.string());

    EXPECT_EQ(config.tape.read_delay_ms, 5);
    EXPECT_EQ(config.tape.write_delay_ms, 6);
}

TEST_F(ConfigLoaderTest, HandlesSpacesAndTabs)
{
    write_config(R"(
        tape.read_delay_ms    =     10
        tape.write_delay_ms   =   20
        tape.move_delay_ms = 1
        tape.rewind_delay_ms = 1

        memory.limit_bytes = 2048
        filesystem.tmp_dir = ./tmp
        log.enabled = true
    )");

    auto config = ConfigLoader::load(path.string());

    EXPECT_EQ(config.tape.read_delay_ms, 10);
    EXPECT_EQ(config.tape.write_delay_ms, 20);
}

TEST_F(ConfigLoaderTest, ThrowsIfNoEqualSign)
{
    write_config(R"(
        tape.read_delay_ms 10
    )");

    EXPECT_THROW(
        ConfigLoader::load(path.string()),
        std::runtime_error
    );
}

TEST_F(ConfigLoaderTest, ThrowsOnEmptyKey)
{
    write_config(R"(
        = 10
    )");

    EXPECT_THROW(
        ConfigLoader::load(path.string()),
        std::runtime_error
    );
}

TEST_F(ConfigLoaderTest, ThrowsOnMissingKey)
{
    write_config(R"(
        tape.read_delay_ms = 1
    )");

    EXPECT_THROW(
        ConfigLoader::load(path.string()),
        std::runtime_error
    );
}

TEST_F(ConfigLoaderTest, ThrowsOnInvalidInt)
{
    write_config(R"(
        tape.read_delay_ms = abc
        tape.write_delay_ms = 1
        tape.move_delay_ms = 1
        tape.rewind_delay_ms = 1

        memory.limit_bytes = 100

        filesystem.tmp_dir = ./tmp
        log.enabled = true
    )");

    EXPECT_THROW(
        ConfigLoader::load(path.string()),
        std::runtime_error
    );
}

TEST_F(ConfigLoaderTest, ThrowsOnInvalidBool)
{
    write_config(R"(
        tape.read_delay_ms = 1
        tape.write_delay_ms = 1
        tape.move_delay_ms = 1
        tape.rewind_delay_ms = 1

        memory.limit_bytes = 100

        filesystem.tmp_dir = ./tmp
        log.enabled = maybe
    )");

    EXPECT_THROW(
        ConfigLoader::load(path.string()),
        std::runtime_error
    );
}

TEST_F(ConfigLoaderTest, SupportsBoolAsZeroOne) {
    write_config(R"(
        tape.read_delay_ms = 10
        tape.write_delay_ms = 20
        tape.move_delay_ms = 30
        tape.rewind_delay_ms = 40

        memory.limit_bytes = 1024

        filesystem.tmp_dir = ./tmp

        log.enabled = 1
    )");

    const AppConfig config =
        ConfigLoader::load(path.string());

    EXPECT_TRUE(config.log.enabled);
}

TEST(ConfigLoaderTests, ThrowsOnMissingKey) {
    EXPECT_THROW(ConfigLoader::load("bad_config.txt"), std::runtime_error);
}
