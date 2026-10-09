#include "drive.hpp"

WheelCommands mixDrive(DriveCommand command){
    WheelCommands wheels{

    };

    wheels.fl = command.cmdForward + command.cmdStrafe - command.cmdTurn;
    wheels.fr = command.cmdForward - command.cmdStrafe + command.cmdTurn;
    wheels.bl = command.cmdForward - command.cmdStrafe - command.cmdTurn;
    wheels.br = command.cmdForward + command.cmdStrafe + command.cmdTurn;


    return wheels;

};