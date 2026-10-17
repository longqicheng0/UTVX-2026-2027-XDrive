#include "util.hpp"
#include <cmath>
#include <numbers>

Vec2 bodyToField(Vec2 bodyVelocity, double theta){
    Vec2 fieldVelocity{};   

    fieldVelocity.x = bodyVelocity.x * std::cos(theta) - bodyVelocity.y * std::sin(theta);
    fieldVelocity.y = bodyVelocity.x * std::sin(theta) + bodyVelocity.y * std::cos(theta);

    return fieldVelocity;
}

Pose integratePose(Pose pose, Vec2 bodyVelocity, double angularVelocity, double dt){
    Vec2 fieldVelocity = bodyToField(bodyVelocity, pose.theta);

    pose.x += fieldVelocity.x * dt;
    pose.y += fieldVelocity.y * dt;
    pose.theta += angularVelocity * dt;

    return pose;
}

double wrapAngles(double angle){
    while (angle > std::numbers::pi){
        angle -= 2 * std::numbers::pi;
    }
    while (angle < -std::numbers::pi){
        angle += 2 * std::numbers::pi;
    }
    return angle;
}

double elapsedSeconds(std::uint32_t startTime, std::uint32_t endTime){
    return (endTime - startTime)/1000.0;
}