#include <iostream>
#include "main/src/drive.hpp"


int main(){
    DriveCommand command{};
    command.cmdForward = 0;
    command.cmdStrafe = 0;
    command.cmdTurn = 0.5;

    WheelCommands wheels = mixDrive(command);

    //std::cout << command.cmdForward; 
    std::cout << wheels.fl << " " << wheels.fr << " " << wheels.bl << " " << wheels.br;

};
