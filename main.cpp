#include <iostream>
#include <algorithm>
#include <cmath>
#include "main/src/drive.hpp"
#include "main/src/Libraries/util.hpp"


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

    Pose pose{};

    pose.theta = 0;

    double dt = 0.1;              // seconds
    double maxLinearSpeed = 1.0;  // metres per second
    double maxTurnSpeed = 1.0;    // radians per second
    double angularVelocity = command.cmdTurn * maxTurnSpeed;

    for (int step = 0; step < 10; step++){
        Vec2 bodyVelocity{};

        bodyVelocity.x = command.cmdStrafe * maxLinearSpeed;
        bodyVelocity.y = command.cmdForward * maxLinearSpeed;

        // fieldVelocity = bodyToField(bodyVelocity, pose.theta);

        // pose.x += fieldVelocity.x * dt;
        // pose.y += fieldVelocity.y * dt;
        // pose.theta += command.cmdTurn * maxTurnSpeed * dt;
        pose = integratePose(pose, bodyVelocity, angularVelocity, dt);

        std::cout << "\nPose: " << pose.x << " " << pose.y << " " << pose.theta << '\n';
        
        



    }

    

        
        







};
