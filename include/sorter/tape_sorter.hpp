#pragma once // sorter/tape_sorter.hpp

#include <vector>
#include <memory>

#include "config/config_structs.hpp"
#include "tape/interface_tape.hpp"
#include "tape/temp_file_tape.hpp"

namespace tape_sort::sorter {

class TapeSorter {
public:
    explicit TapeSorter(const config::AppConfig& config);

    void sort(tape::ITape& input, tape::ITape& output);

private:
    using RunPtr = std::unique_ptr<tape::TempFileTape>;

private:
    std::vector<RunPtr> create_runs(tape::ITape& input);

    void merge_runs(std::vector<RunPtr>& runs, tape::ITape& output);

    size_t compute_chunk_size();
    std::vector<int32_t> read_chunk(tape::ITape& input, size_t chunk_size);
    void write_run_to_tape(tape::TempFileTape& tape, const std::vector<int32_t>& data);
    std::string make_run_path(const std::string& dir, size_t index);

private:
    config::AppConfig config_;
};

} // namespace tape_sort::sorter
