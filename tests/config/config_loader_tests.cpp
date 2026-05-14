#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "config/config_loader.hpp"

namespace fs = std::filesystem;

namespace tape_sort::config::test {

class ConfigLoaderTest : public ::testing::Test {
protected:
    fs::path temp_file_;

    void TearDown() override {
        if (!temp_file_.empty() && fs::exists(temp_file_)) {
            fs::remove(temp_file_);
        }
    }

    void write_config(const std::string& content) {
        temp_file_ = fs::temp_directory_path() / "test_config.cfg";

        std::ofstream out(temp_file_);

        ASSERT_TRUE(out.is_open());

        out << content;
    }
};

TEST_F(ConfigLoaderTest, LoadsValidConfig) {
    write_config(R"(
        tape.read_delay_ms = 10
        tape.write_delay_ms = 20
        tape.move_delay_ms = 30
        tape.rewind_delay_ms = 40

        memory.limit_bytes = 1024

        filesystem.tmp_dir = ./tmp

        log.enabled = true
    )");

    const AppConfig config =
        ConfigLoader::load(temp_file_.string());

    EXPECT_EQ(config.tape.read_delay_ms, 10);
    EXPECT_EQ(config.tape.write_delay_ms, 20);
    EXPECT_EQ(config.tape.move_delay_ms, 30);
    EXPECT_EQ(config.tape.rewind_delay_ms, 40);

    EXPECT_EQ(config.memory.limit_bytes, 1024);

    EXPECT_EQ(config.filesystem.tmp_dir, "./tmp");

    EXPECT_TRUE(config.log.enabled);
}

TEST_F(ConfigLoaderTest, SupportsComments) {
    write_config(R"(
        // full line comment

        tape.read_delay_ms = 1 // comment
        tape.write_delay_ms = 2
        tape.move_delay_ms = 3
        tape.rewind_delay_ms = 4

        memory.limit_bytes = 2048

        filesystem.tmp_dir = /tmp/data

        log.enabled = false
    )");

    const AppConfig config =
        ConfigLoader::load(temp_file_.string());

    EXPECT_EQ(config.tape.read_delay_ms, 1);
    EXPECT_FALSE(config.log.enabled);
}

TEST_F(ConfigLoaderTest, SupportsRandomOrder) {
    write_config(R"(
        log.enabled = true

        filesystem.tmp_dir = ./cache

        tape.move_delay_ms = 30
        tape.read_delay_ms = 10

        memory.limit_bytes = 9999

        tape.rewind_delay_ms = 40
        tape.write_delay_ms = 20
    )");

    const AppConfig config =
        ConfigLoader::load(temp_file_.string());

    EXPECT_EQ(config.tape.read_delay_ms, 10);
    EXPECT_EQ(config.tape.write_delay_ms, 20);
    EXPECT_EQ(config.tape.move_delay_ms, 30);
    EXPECT_EQ(config.tape.rewind_delay_ms, 40);

    EXPECT_EQ(config.memory.limit_bytes, 9999);

    EXPECT_EQ(config.filesystem.tmp_dir, "./cache");

    EXPECT_TRUE(config.log.enabled);
}

TEST_F(ConfigLoaderTest, ThrowsIfFileDoesNotExist) {
    EXPECT_THROW(
        ConfigLoader::load("definitely_not_existing.cfg"),
        std::runtime_error
    );
}

TEST_F(ConfigLoaderTest, ThrowsIfKeyMissing) {
    write_config(R"(
        tape.read_delay_ms = 10
        tape.write_delay_ms = 20
        tape.move_delay_ms = 30

        memory.limit_bytes = 1024

        filesystem.tmp_dir = ./tmp

        log.enabled = true
    )");

    EXPECT_THROW(
        ConfigLoader::load(temp_file_.string()),
        std::runtime_error
    );
}

TEST_F(ConfigLoaderTest, ThrowsOnInvalidInteger) {
    write_config(R"(
        tape.read_delay_ms = abc
        tape.write_delay_ms = 20
        tape.move_delay_ms = 30
        tape.rewind_delay_ms = 40

        memory.limit_bytes = 1024

        filesystem.tmp_dir = ./tmp

        log.enabled = true
    )");

    EXPECT_THROW(
        ConfigLoader::load(temp_file_.string()),
        std::runtime_error
    );
}

TEST_F(ConfigLoaderTest, ThrowsOnInvalidBool) {
    write_config(R"(
        tape.read_delay_ms = 10
        tape.write_delay_ms = 20
        tape.move_delay_ms = 30
        tape.rewind_delay_ms = 40

        memory.limit_bytes = 1024

        filesystem.tmp_dir = ./tmp

        log.enabled = maybe
    )");

    EXPECT_THROW(
        ConfigLoader::load(temp_file_.string()),
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
        ConfigLoader::load(temp_file_.string());

    EXPECT_TRUE(config.log.enabled);
}

} // namespace tape_sort::config::test
