#include <riplib/riplib.h>
#include <wallaby/wallaby.h> // for IR calibration

using namespace rip;


int main() {
    RIP::initialize("/home/pi/programs/gcer2023/secondbot/config.json");
    LOG->info("This is DEFAULT");

    /// region Constants {...}
    const int winch_ret_1 = RIP::get_config("winch/positions/ret_1").get<int>();
    const int winch_ret_2 = RIP::get_config("winch/positions/ret_2").get<int>();
    const int winch_ret_post_3 = RIP::get_config("winch/positions/ret_post_3").get<int>();

    const int winch_key_low = RIP::get_config("winch/positions/key_low").get<int>();
    const int winch_key_medium = RIP::get_config("winch/positions/key_medium").get<int>();
    const int winch_key_tall = RIP::get_config("winch/positions/key_tall").get<int>();

    const int winch_pre_alarm = RIP::get_config("winch/positions/pre_alarm").get<int>();
    const int winch_bottom = RIP::get_config("winch/positions/bottom").get<int>();

    const double finger_short = RIP::get_config("finger/positions/short").get<double>();
    const double finger_medium = RIP::get_config("finger/positions/medium").get<double>();
    const double finger_long = RIP::get_config("finger/positions/long").get<double>();
    // endregion


    /// region Strategy {...}
    auto do_key = [](int num) {
        LOG->info("Do Key {}? (A/B)", num);
        while (RIP::is_running()) {
            if (::a_button()) {
                LOG->info("YES");
                sleep(500);
                return true;
            } else if (::b_button()) {
                LOG->info("NO");
                sleep(500);
                return false;
            }
            sleep(1);
        }
        return false;
    };

    bool do_key6 = do_key(6);
    bool do_key5 = do_key(5);
    bool do_key4 = do_key(4);
    bool do_key3 = do_key(3);
    bool do_key2 = do_key(2);
    bool do_key1 = do_key(1);
    // endregion


    /// region Calibration {...}
    double ir_black = 3751;
    double ir_white = 1619;
    double ir_gray = 3106.5;

    auto &cfg = RIP::get_config_bad();
    auto left_sens = Sensor("movement/left_sensor");
    auto right_sens = Sensor("movement/right_sensor");
    bool is_calibrated = false;
    while (RIP::is_running() && !is_calibrated) {
        LOG->warn("Calibration -> make sure both sensors are on the correct color");

        // ir_black
        ir_black = 0;
        LOG->info("Press to calibrate IR BLACK LINE");
        wait_for(::push_button);
        for (int i{0}; i < 3; ++i) {
            ir_black += (left_sens.read() + right_sens.read()) * 0.5;
            sleep(50);
        }
        ir_black /= 3;

        LOG->info("IR BLACK LINE is {}", ir_black);
        sleep(1000);

        // ir_gray
        ir_gray = 0;
        LOG->info("Press to calibrate IR GRAY");
        wait_for(::push_button);
        for (int i{0}; i < 3; ++i) {
            ir_gray += right_sens.read();
            sleep(50);
        }
        ir_gray /= 3;
        LOG->info("IR GRAY is {}", ir_gray);
        sleep(1000);

        // ir_white
        ir_white = 0;
        LOG->info("Press to calibrate IR WHITE");
        wait_for(::push_button);
        for (int i{0}; i < 3; ++i) {
            ir_white += (left_sens.read() + right_sens.read()) * 0.5;
            sleep(50);
        }
        ir_white /= 3;
        LOG->info("IR WHITE is {}", ir_white);
        sleep(1000);

        is_calibrated = true;

        // recalibrate?
        auto wait_start = now();
        while (RIP::is_running() && time_diff(now(), wait_start) < 5000) {
            if (::push_button()) {
                is_calibrated = false;
                sleep(1000);
                break;
            }
            sleep(1);
        }
    }

    cfg["movement"]["white_value"] = ir_white;
    cfg["movement"]["black_value"] = ir_black;
    // endregion


    /// region Objects {...}
    auto &movement = Movement::get();
    auto winch = Motor("winch");
    auto finger = Servo("finger");

    auto correction_gyro = Gyro("gyro"); // copy of default gyro
    // endregion


    /// region Functions {...}
    auto sync_to = [](double sync_time) -> double {
        double start = RIP::get_game_time();
        if (start < sync_time) {
            LOG->info("Syncing: {:0.2f} -> {:0.2f} ({:0.2f} seconds)", start, sync_time, sync_time - start);
            wait_for([&] {
                return RIP::get_game_time() > sync_time;
            });
            return sync_time - start;
        } else {
            LOG->error("Bad sync: {:0.2f} -> {:0.2f} ({:0.2f} seconds)!", start, sync_time, sync_time - start);
            return 0;
        }
    };

    auto winch_move_for = [&](int time_ms, double speed) {
        winch.set_speed(speed);
        sleep(time_ms);
        winch.freeze();
    };

    auto movement_drive_to_gray_right = [&](bool forwards = true, double speed = 1.0, bool accelerate = true) {
        double threshold = (ir_gray + ir_white) * 0.5;
        movement.drive_until(forwards, [&] { return movement.get_right_sensor() > threshold; }, speed, accelerate);
    };

    auto movement_drive_to_black = [&](bool forwards = true, double speed = 1.0, bool accelerate = true) {
        movement.drive_until(forwards, [&] {
            return movement.is_black(movement.get_left_sensor()) ||
                   movement.is_black(movement.get_right_sensor());
        }, speed, accelerate);
    };
    // endregion


    /// region Robot Setup {...}
    // busy-call gyro to correct overflow
    std::thread([&] {
        while (RIP::is_running()) {
            correction_gyro.read();
            sleep(50);
        }
    }).detach();

    new Task([&] { finger.move_to(finger_short, 1500); });
    movement.drive_raw(-1);
    movement.drive(3);
    Task::join_all();
    // endregion

    RIP::await_game_start(false);
    double total_waiting_time = 0;
    double error_angle;

    /// region Reverse Engineering Tool {...}
    LOG->info("--- REVERSE ENGINEERING TOOL ({}) ---", RIP::get_game_time());
    // move to RET
    movement.drive(8);
    movement.turn(-90);
    movement.drive(-95, 1, 0, 1.0, true, false);
    movement.drive_raw(-15, 0.7);

    // ram ring backwards
    movement.drive(10);
    movement.turn(90);
    movement.drive(-20, 0, 0, 1.0, true, false);

    // grab ring
    movement.drive(5);
    movement.turn(90);
    movement.drive(-13.5);
    movement.turn(55);
    correction_gyro.reset();
    // gently
    movement.drive(28, 0, 0, 1.0, true, false);

    // correct position
    movement.drive(-5);
    error_angle = correction_gyro.read();
    LOG->info("Correcting position by {:0.2f}°", -error_angle);
    movement.turn(-error_angle);

    // tier 1
    winch.move_to(winch_ret_1);
    movement.turn(90 - 55 + 39);

    // tier 2
    movement.drive(-7, 0, 0, 0.5);
    winch.move_to(winch_ret_2);

    // wait for Main
    total_waiting_time += sync_to(40);

    // tier 3
    movement.drive(-13.5, 0, 0, 0.5, true, false);
    winch_move_for(2500, 1.0);

    // move ring to safety
    movement.drive(-7);
    movement.turn(-10);
    movement.drive(-12, 0, 0, 1.0, true, false);

    // lose ring
    movement.turn(-39 + 10 - 90);
    movement.drive(-10, 0, 0, 1.0, false, true);
    // endregion


    /// region Alarm {...}
    LOG->info("--- ALARM ({})---", RIP::get_game_time());

    // align at center line
    new Task([&] { winch.move_to(winch_pre_alarm); });
    movement.turn(-90);
    movement.drive(-20);
    movement.align_at_next_line(true);

    // move to Alarm
    movement.drive(13, 0, 0, 1.0, true, true);
    movement.turn(90);
    movement.drive(25, 0, 0, 1.0, true, false);
    Task::join_all();

    // flip switch
    winch_move_for(1000, 1.0);
    // endregion


    /// region Encryption Key 6 {...}
    LOG->info("--- ENCRYPTION KEY 6 ({}) ---", RIP::get_game_time());
    // bump at middle border
    new Task([&] {
        finger.move_to(finger_medium, 1000);
        finger.move_to(finger_short, 700);
    });
    new Task([&] { winch.move_to(winch_key_low); });
    movement.drive(-25);
    movement.turn(-90);
    movement.drive(50, 0, 0, 1.0, true, false);
    movement.drive_raw(20, 0.9);

    // record reference point for driving next to keys
    sleep(500);
    correction_gyro.reset();
    double reference_offset = 86.5;

    // bump in corner
    movement.drive(-7);
    movement.turn(-90);
    movement.drive(-40, 0, 0, 0.7, true, false);
    Task::join_all();

    if (do_key6) {
        // pull key
        movement.drive(5, 0, 0, 1.0, false, false);
        finger.move_to(finger_long, 500);
        movement.drive(-5, 0, 0, 1.0, false, false);
        new Task([&] { winch.move_to(winch_key_low - 100); });
        finger.move_to(finger_short, 750);
        Task::join_all();

        // release key
        movement.drive(7, 0, 0, 1.0, false, false);
        finger.move_to(finger_medium, 500);
        finger.move_to(finger_short);
    }

    // correct angle
    error_angle = correction_gyro.read() + reference_offset;
    LOG->info("Correcting position by {:0.2f}°", -error_angle);
    movement.turn(-error_angle);
    new Task([&] { winch.move_to(winch_key_medium); });
    // endregion

    total_waiting_time += sync_to(91.5);

    /// region Encryption Key 5 {...}
    LOG->info("--- ENCRYPTION KEY 5 ({}) ---", RIP::get_game_time());
    // move to key, prepare winch
    movement_drive_to_gray_right(true, 1.0, false);
    movement.drive(25.5, 0, 0, 1.0, false, true);
    Task::join_all();

    if (do_key5) {
        // pull key
        finger.move_to(finger_long, 500);
        movement.drive(5, 0, 0, 1.0, false, false);
        movement.turn(-correction_gyro.read() - 90); // straighten bot
        new Task([&] { winch.move_to(winch_key_medium - 100); });
        finger.move_to(finger_short, 750);
        Task::join_all();

        // release key
        movement.drive(-8, 0, 0, 1.0, false, false);
        finger.move_to(finger_medium, 500);
        finger.move_to(finger_short);
    }

    // correct angle
    error_angle = correction_gyro.read() + reference_offset;
    LOG->info("Correcting position by {:0.2f}°", -error_angle);
    movement.turn(-error_angle);
    // endregion


    /// region Encryption Key 4 {...}
    LOG->info("--- ENCRYPTION KEY 4 ({}) ---", RIP::get_game_time());
    // move to key, prepare winch
    new Task([&] { winch.move_to(winch_key_tall); });
    movement_drive_to_gray_right(true, 1.0, false);
    movement.drive(25.5, 0, 0, 1.0, false, true);
    Task::join_all();

    if (do_key4) {
        // pull key
        finger.move_to(finger_long, 500);
        movement.drive(5, 0, 0, 1.0, false, false);
        movement.turn(-correction_gyro.read() - 90); // straighten bot
        new Task([&] { winch.move_to(winch_key_tall - 100); });
        finger.move_to(finger_short, 750);
        Task::join_all();

        // release key
        movement.drive(-8, 0, 0, 1.0, false, false);
        finger.move_to(finger_medium, 500);
        finger.move_to(finger_short);
    }

    // correct angle
    error_angle = correction_gyro.read() + reference_offset;
    LOG->info("Correcting position by {:0.2f}°", -error_angle);
    movement.turn(-error_angle);
    // endregion


    /// region Encryption Key 3 {...}
    LOG->info("--- ENCRYPTION KEY 3 ({}) ---", RIP::get_game_time());
    // move to key
    new Task([&] { winch.move_to(winch_key_tall); });
    movement_drive_to_black(true, 1.0, false);
    movement.drive(25.5, 0, 0, 1.0, false, true);
    Task::join_all();

    if (do_key3) {
        // pull key
        finger.move_to(finger_long, 500);
        movement.drive(5, 0, 0, 1.0, false, false);
        movement.turn(-correction_gyro.read() - 90); // straighten bot
        new Task([&] { winch.move_to(winch_key_tall - 100); });
        finger.move_to(finger_short, 750);
        Task::join_all();

        // release key
        movement.drive(-8, 0, 0, 1.0, false, false);
        finger.move_to(finger_medium, 500);
        finger.move_to(finger_short);
    }

    // correct angle
    error_angle = correction_gyro.read() + reference_offset;
    LOG->info("Correcting position by {:0.2f}°", -error_angle);
    movement.turn(-error_angle);
    // endregion


    /// region Encryption Key 2 {...}
    LOG->info("--- ENCRYPTION KEY 2 ({}) ---", RIP::get_game_time());
    // move to key
    movement_drive_to_gray_right(true, 1.0, false);
    new Task([&] { winch.move_to(winch_key_medium); });
    movement.drive(25.5, 0, 0, 1.0, false, true);
    Task::join_all();

    if (do_key2) {
        // pull key
        finger.move_to(finger_long, 500);
        movement.drive(5, 0, 0, 1.0, false, false);
        movement.turn(-correction_gyro.read() - 90); // straighten bot
        new Task([&] { winch.move_to(winch_key_medium - 100); });
        finger.move_to(finger_short, 750);
        Task::join_all();

        // release key
        movement.drive(-8, 0, 0, 1.0, false, false);
        finger.move_to(finger_medium, 500);
        finger.move_to(finger_short);
    }

    // correct angle
    error_angle = correction_gyro.read() + reference_offset;
    LOG->info("Correcting position by {:0.2f}°", -error_angle);
    movement.turn(-error_angle);
    // endregion


    /// region Encryption Key 1 {...}
    LOG->info("--- ENCRYPTION KEY 1 ({}) ---", RIP::get_game_time());
    if (do_key1) {
        // move to key
        movement_drive_to_gray_right(true, 1.0, false);
        new Task([&] { winch.move_to(winch_key_low); });
        movement.drive(25.5, 0, 0, 1.0, false, true);

        // pull key
        finger.move_to(finger_long, 500);
        Task::join_all();
        movement.drive(5, 0, 0, 1.0, false, false);
        movement.turn(-correction_gyro.read() - 90); // straighten bot
        new Task([&] { winch.move_to(winch_key_low - 100); });
        finger.move_to(finger_short, 750);
        Task::join_all();
        movement.drive(-8, 0, 0, 1.0, false, false);
    }
    // endregion

    RIP::shutdown();
}
