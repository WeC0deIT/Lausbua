#include <riplib/riplib.h>

using namespace rip;

int main() {
    RIP::initialize("/home/pi/programs/examples/secondbot/config.json");
    auto &movement = Movement::get();
    auto button1 = Button("button1");
    auto button2 = Button("button2");
    auto button3 = Button("button3");
    auto button4 = Button("button");
    auto servo1 = Servo("servo1");

    //Await Game Start (remove true when activating using light instead of the button); Deactivated for the time being
    //RIP::await_game_start(true);

    // --- Botball Strategy ---
    //Open Airlock on set path; return to starting position and bring rocks to rock heap; return to starting position and close airlock;

    //Line up at start of game.
    movement.set_speed(-1, -1);
    wait_for([&] {return button1.is_pressed() && button2.is_pressed(); });
    movement.freeze();

    //Collecting the rocks for the rock heap. (Currently testing)
    movement.drive(40);
    movement.turn(-15);
    //first rock collected
    movement.drive_until(true, [&] {return button4.is_pressed();});
    movement.turn(18);
    movement.drive(10);
    //second rock collected
    //code below to position the second bot, to allow the orange poms to go under the arms
    movement.turn(-17);
    movement.drive(10);
    movement.turn(17);
    movement.drive(90);
    movement.turn(15);
    movement.drive(10);
    //third and final rock collected
    movement.turn(-45);
    movement.drive(15);

    //Return to proper position in order to open AirLock
    movement.drive(-35);
    movement.turn(35);
    movement.drive_until(false, [&] {return button1.is_pressed() && button2.is_pressed();});
    movement.drive(70);
    movement.turn(85);

    //Line up to reach Airlock the same way consistently.
    movement.set_speed(-1, -1);
    wait_for([&] {return (button1.is_pressed() && button2.is_pressed())|| button3.is_pressed();});
    movement.freeze();
    servo1.move_to(-30);
    movement.drive(30);
    movement.turn(-140);

    //Driving into Airlock.
    movement.drive(15);
    servo1.move_to(20);

    //Opening AirLock.
    movement.drive(-15);

    //Add here: Have the second bot drive to the lava tube, so the mainbot can use the container to store the poms.

    //Lineup to properly close.
    movement.drive(15);
    servo1.move_to(-30);
    movement.drive(-15);

    RIP::shutdown();
}