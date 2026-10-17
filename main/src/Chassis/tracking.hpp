#pragma once

struct SensorSample{
    double rightTrackingWheelDist;
    double forwardTrackingWheelDist;
    double IMUHeading;
    double timestamp;
    bool isValid;
}