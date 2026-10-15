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

WheelCommands normWheelCommands(WheelCommands wheels);

WheelCommands mixDrive(DriveCommand command);