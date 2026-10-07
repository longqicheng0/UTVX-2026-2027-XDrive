#pragma once


struct DriveCommand{
    double cmdForward;
    double cmdStrafe;
    double cmdTurn;
};

struct WheelCommands{
    double fl;
    double fr;
    double bl;
    double br;
};

WheelCommands mixDrive(DriveCommand command);