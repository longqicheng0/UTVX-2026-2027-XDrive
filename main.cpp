#include <iostream>
#include <algorithm>
#include <cmath>
#include "main/src/drive.hpp"


int main(){
    DriveCommand command{};
    std::cout << "Enter forward, strafe, turn: ";
    std::cin >> command.cmdForward;
    std::cout << "Forward is " << command.cmdForward << "\n";
    std::cin >> command.cmdStrafe;
    std::cout << "Strafe is " << command.cmdStrafe << "\n";
    std::cin >> command.cmdTurn;
    std::cout << "Turn is " << command.cmdTurn << "\n";

    WheelCommands wheels = mixDrive(command);
    // double div = std::max({1.0, std::abs(wheels.fl), std::abs(wheels.fr), std::abs(wheels.bl), std::abs(wheels.br)});
    // wheels = {wheels.fl / div, wheels.fr / div, wheels.bl / div, wheels.br / div};

    wheels = normWheelCommands(wheels);

    //std::cout << command.cmdForward; 
    std::cout << wheels.fl << " " << wheels.fr << " " << wheels.bl << " " << wheels.br;


};
