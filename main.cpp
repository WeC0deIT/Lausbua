#include <riplib/riplib.h>

using namespace rip;

int main() {
    RIP::initialize("/home/pi/programs/examples/secondbot/config.json");

    // --- objects --- //
    auto &movement = Movement::get();

    // --- robot setup --- //
    movement.drive_raw(-5);

    RIP::await_game_start();

    // --- botball strategy --- //
    movement.align_at_next_line();

    new Task([&] {
        for (int i = 0; i < 5; i++) {
            LOG->info("Example thread doing something in parallel, iteration {}", i);
            sleep(1000);
        }
    });
    movement.drive(40, 15, 20);
    Task::join_all();

    movement.drive(-20);

    LOG->info("Waiting for 15 second mark at {:2f}s", RIP::get_game_time());
    util::wait_for([&] { // do nothing and wait until 15 seconds after game start
        return RIP::get_game_time() > 15;
    });

    movement.turn(-110);
    movement.align_at_next_line();
    movement.drive(50);
    movement.turn(-90);
    movement.drive(30);
    movement.turn(-90);
    movement.drive(20);
    movement.align_at_next_line();
    movement.drive(25);
    movement.turn(-90);
    movement.drive_raw(-20); // "bump"

    RIP::shutdown(); // forgetting this line will result in runtime errors!
}
