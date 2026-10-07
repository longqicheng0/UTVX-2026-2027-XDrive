#include <iostream>
#include "main/src/drive.hpp"


int main(){
    double cmdForward = 0.5;

    std::cout << cmdForward; //Print sth: std::cout << <variable>

    DriveCommand command{};
    command.cmdForward = 0.5;

    std::cout << command.cmdForward; 

};
