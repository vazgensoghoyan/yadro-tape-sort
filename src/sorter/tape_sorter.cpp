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

    input.rewind();

    while (true) {
        auto buffer = read_chunk(input, chunk_size);
        if (buffer.empty())
            break;

        std::sort(buffer.begin(), buffer.end());

        auto run = std::make_unique<TempFileTape>(config_.filesystem.tmp_dir, buffer.size());

        write_run_to_tape(*run, buffer);

        runs.push_back(std::move(run));
    }

    LOG_INFO("Total temp tapes created: {}", runs.size());

    return runs;
}

// MERGE

namespace {

struct RunState {
    TempFileTape* tape;
    int32_t value;
    bool finished = false;
};

struct Cmp {
    bool operator()(const RunState* a, const RunState* b) const {
        return a->value > b->value;
    }
};

}

void TapeSorter::merge_runs(std::vector<RunPtr>& runs, ITape& output) {
    output.rewind();

    std::vector<RunState> states;
    states.reserve(runs.size());

    std::priority_queue<RunState*, std::vector<RunState*>, Cmp> pq;

    for (auto& run : runs) {
        run->rewind();

        if (!run->is_eof()) {
            RunState state;
            state.tape = run.get();
            state.value = run->read();

            states.push_back(state);
            pq.push(&states.back());
        }
    }

    while (!pq.empty()) {
        RunState* cur = pq.top();
        pq.pop();

        output.write(cur->value);
        output.move_right();

        cur->tape->move_right();

        if (!cur->tape->is_eof()) {
            cur->value = cur->tape->read();
            pq.push(cur);
        }
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
