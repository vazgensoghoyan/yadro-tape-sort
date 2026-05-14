#include "sorter/tape_sorter.hpp"

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <queue>
#include <limits>

#include "utils/logger.hpp"

using namespace tape_sort::sorter;
using namespace tape_sort::tape;
using namespace tape_sort::config;

namespace fs = std::filesystem;

TapeSorter::TapeSorter(const AppConfig& config) : config_(config) {
    
}

void TapeSorter::sort(ITape& input, ITape& output) {
    auto runs = create_runs(input);
    merge_runs(runs, output);
    
    LOG_INFO("Sorting completed successfully");
}

// CHUNKS SORT AND TEMP TAPES

std::vector<TapeSorter::RunPtr> TapeSorter::create_runs(ITape& input) {
    const size_t chunk_size = compute_chunk_size();

    LOG_DEBUG("Chunk size is {}", chunk_size);

    fs::create_directories(config_.filesystem.tmp_dir);

    std::vector<RunPtr> runs;
    size_t run_index = 0;

    input.rewind();

    while (true) {
        auto buffer = read_chunk(input, chunk_size);
        if (buffer.empty())
            break;

        std::sort(buffer.begin(), buffer.end());

        auto path = make_run_path(config_.filesystem.tmp_dir, run_index++);
        auto run = std::make_unique<TempFileTape>(path, buffer.size());

        write_run_to_tape(*run, buffer);

        LOG_DEBUG("Created temp tape {} with size {}", path, buffer.size());

        runs.push_back(std::move(run));
    }

    LOG_INFO("Total temp tapes created: {}", runs.size());

    return runs;
}

// MERGE

void TapeSorter::merge_runs(std::vector<RunPtr>& runs, ITape& output) {
    output.rewind();

    std::vector<bool> finished(runs.size(), false);

    for (auto& run : runs)
        run->rewind();

    while (true) {
        int32_t min_value = std::numeric_limits<int32_t>::max();
        size_t min_index = runs.size();

        for (size_t i = 0; i < runs.size(); ++i) {
            if (finished[i])
                continue;

            auto& tape = *runs[i];

            if (tape.is_eof()) {
                finished[i] = true;
                continue;
            }

            int32_t value = tape.read();

            if (value < min_value) {
                min_value = value;
                min_index = i;
            }
        }

        if (min_index == runs.size())
            break;

        output.write(min_value);
        output.move_right();

        runs[min_index]->move_right();
    }
}

// HELPERS

size_t TapeSorter::compute_chunk_size() {
    const size_t cell_size = sizeof(int32_t);
    const size_t chunk = config_.memory.limit_bytes / cell_size;

    if (chunk == 0)
        throw std::runtime_error("Memory limit too small");

    return chunk;
}

std::vector<int32_t> TapeSorter::read_chunk(ITape& input, size_t chunk_size) {
    std::vector<int32_t> buffer;
    buffer.reserve(chunk_size);

    while (buffer.size() < chunk_size && !input.is_eof()) {
        buffer.push_back(input.read());
        input.move_right();
    }

    return buffer;
}

void TapeSorter::write_run_to_tape(TempFileTape& tape, const std::vector<int32_t>& data) {
    for (auto number : data) {
        tape.write(number);
        tape.move_right();
    }

    tape.rewind();
}

std::string TapeSorter::make_run_path(const std::string& dir, size_t index) {
    return dir + "/run_" + std::to_string(index) + ".bin";
}
