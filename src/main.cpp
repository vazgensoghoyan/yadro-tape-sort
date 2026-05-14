#include "app/app.hpp"

using namespace tape_sort;

int main(int argc, char** argv) {

    app::App app;
    app::ExitCode exit_code = app.run(argc, argv);

    return static_cast<int>(exit_code);
}
