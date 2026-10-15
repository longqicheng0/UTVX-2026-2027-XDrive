#include "drive.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

WheelCommands mixDrive(DriveCommand command){
    WheelCommands wheels{};

    wheels.fl = command.cmdForward + command.cmdStrafe - command.cmdTurn;
    wheels.fr = command.cmdForward - command.cmdStrafe + command.cmdTurn;
    wheels.bl = command.cmdForward - command.cmdStrafe - command.cmdTurn;
    wheels.br = command.cmdForward + command.cmdStrafe + command.cmdTurn;

    return wheels;
};

WheelCommands normWheelCommands(WheelCommands wheels){
    double div = std::max({1.0, std::abs(wheels.fl), std::abs(wheels.fr), std::abs(wheels.bl), std::abs(wheels.br)});
    wheels = {wheels.fl / div, wheels.fr / div, wheels.bl / div, wheels.br / div};
    return wheels;
};