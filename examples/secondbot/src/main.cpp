#include <riplib/riplib.h>

using namespace rip;

int main() {
    RIP::initialize("/home/pi/programs/examples/Lausbua/secondbot/config.json");
    auto &movement = Movement::get();
    auto button1 = Button("button1");
    auto button2 = Button("button2");
    auto servo1 = Servo("servo1");
    auto sensor1 = Sensor("sensor1");

    //Await Game Start (remove true when activating using light instead of the button); Deactivated for the time being
    //RIP::await_game_start(true);

    // --- Botball Strategy ---
    //Open Airlock on set path; return to starting position and bring rocks to rock heap; return to starting position and close airlock;

    //Line up at start of game.
    movement.set_speed(-1, -1);
    wait_for([&] {return button1.is_pressed() && button2.is_pressed();});
    movement.freeze();

    //needed to reach the AirLock
    movement.drive(20);
    movement.drive(35, 0, -45);
    servo1.move_to(-30);
    movement.drive(35);

    //Open AirLock
    servo1.move_to(20);
    movement.drive(-15);
    servo1.move_to(-70);
    movement.drive(-15);
    movement.turn(-55);

    //Reach the Lava Area Tube
    movement.drive_until(false, [&] {return button1.is_pressed() && button2.is_pressed();});
    movement.freeze();
    movement.drive(5);
    movement.turn(-90);
    movement.drive_until(false, [&] {return button1.is_pressed() && button2.is_pressed();});
    movement.freeze();

    //To Add: Have the Bot wait until the main bot is done loading the poms into the container.

    //Reach the AirLock again to unload and close;
    //maybe change to make it faster and more effective
    movement.set_speed(1, 1);
    wait_for([&] {return sensor1.read() > 3300;});
    movement.turn(-90);
    movement.drive(50);
    movement.turn(90);

    //To Add: Have the Bot wait until the main bot is done loading poms into the airlock. (change angle or position as needed)

    //close AirLock
    movement.drive(10);
    movement.turn(25);
    movement.drive(50, 0, -40);
    movement.drive(-10);
    movement.turn(-25);
    servo1.move_to(20);
    movement.drive(30);

    /*
    movement.drive(20);
    movement.turn(90);
    movement.set_speed(-1, -1);
    wait_for([&] {return button1.is_pressed() && button2.is_pressed();});
    movement.drive(52);
    movement.turn(90);
    movement.drive_until(false, [&] {return button1.is_pressed() && button2.is_pressed();});
    movement.drive(35);
    movement.turn(-133);
    servo1.move_to(-50);
    movement.drive(10);
    servo1.move_to(20);
    movement.drive(35);
    */

    //Reach the Equipment.
    servo1.move_to(-50);
    movement.drive(-20);
    movement.turn(-45);
    movement.set_speed(-1, -1);
    wait_for([&] {return button1.is_pressed() && button2.is_pressed();});
    movement.turn(90);
    movement.set_speed(1, 1);
    wait_for([&] {return sensor1.read() > 3300;});
    movement.turn(-90);

    //To Add: Wait until main bot flips switch to catch the equipment.

    movement.turn(-90);
    movement.drive(10);
    movement.set_speed(-1, -1);
    wait_for([&] {return button1.is_pressed() && button2.is_pressed();});

    //To Add:: wait until the main bot has unloaded the equipment into Lava Tube.

    movement.drive(10);
    movement.turn(90);
    movement.drive(50);
    movement.turn(90);
    movement.set_speed(-1, -1);
    wait_for([&] {return button1.is_pressed() && button2.is_pressed();});

    RIP::shutdown();
}