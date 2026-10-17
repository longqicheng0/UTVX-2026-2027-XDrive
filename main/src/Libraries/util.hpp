#pragma once
#include <cstdint>

struct Pose{
    double x;
    double y;
    double theta;
};

struct Vec2{
    double x;
    double y;
};

Vec2 bodyToField(Vec2 bodyVelocity, double theta);

Pose integratePose(Pose pose, Vec2 bodyVelocity, double angularVelocity, double dt);

double wrapAngles(double angle);

double elapsedSeconds(std::uint32_t startTime, std::uint32_t endTime);