#ifndef CALCULATIONS_H
#define CALCULATIONS_H

struct TrafficInput {
    double north;
    double south;
    double east;
    double west;
    double maxDesiredWait;
};

struct SignalPlan {
    double cycleLength;
    double nsGreen;
    double ewGreen;
    double nsDelay;
    double ewDelay;
    double nsQueue;
    double ewQueue;
    double overallDelay;
};

double clampValue(double value, double low, double high);
double estimateCycleLength(const TrafficInput& data);
SignalPlan makeRecommendedPlan(const TrafficInput& data);
SignalPlan makePlan(const TrafficInput& data, double cycleLength);
double estimateAverageDelay(double flow, double green, double cycle);
double estimateQueue(double flow, double green, double cycle);

#endif
