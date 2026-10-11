#include "calculations.h"

#include <algorithm>

namespace {
    // Simplified classroom assumptions:
    const double LOST_TIME_PER_CYCLE = 6.0; // 3 seconds per phase
    const double MIN_GREEN = 10.0;          // seconds per phase
    const double SATURATION_FLOW = 1800.0;   // vehicles/hour for each combined group
}

double clampValue(double value, double low, double high) {
    return std::max(low, std::min(value, high));
}

double estimateCycleLength(const TrafficInput& data) {
    double totalFlow = data.north + data.south + data.east + data.west;

    // Heuristic for a student project, not a standard engineering design formula.
    // Cycle increases with flow, but stays between 60 and 120 seconds.
    return clampValue(60.0 + totalFlow / 25.0, 60.0, 120.0);
}

SignalPlan makeRecommendedPlan(const TrafficInput& data) {
    return makePlan(data, estimateCycleLength(data));
}

SignalPlan makePlan(const TrafficInput& data, double cycleLength) {
    SignalPlan plan{};
    plan.cycleLength = clampValue(cycleLength, 40.0, 180.0);

    double nsFlow = data.north + data.south;
    double ewFlow = data.east + data.west;
    double totalFlow = nsFlow + ewFlow;

    double totalGreen = plan.cycleLength - LOST_TIME_PER_CYCLE;
    double nsGreen;

    if (totalFlow <= 0.0) {
        nsGreen = totalGreen / 2.0;
    } else {
        nsGreen = totalGreen * nsFlow / totalFlow;
    }

    // Keep at least 10 seconds for each phase when the cycle allows it.
    nsGreen = clampValue(nsGreen, MIN_GREEN, totalGreen - MIN_GREEN);
    double ewGreen = totalGreen - nsGreen;

    plan.nsGreen = nsGreen;
    plan.ewGreen = ewGreen;

    plan.nsDelay = estimateAverageDelay(nsFlow, nsGreen, plan.cycleLength);
    plan.ewDelay = estimateAverageDelay(ewFlow, ewGreen, plan.cycleLength);
    plan.nsQueue = estimateQueue(nsFlow, nsGreen, plan.cycleLength);
    plan.ewQueue = estimateQueue(ewFlow, ewGreen, plan.cycleLength);

    if (totalFlow > 0.0) {
        plan.overallDelay =
            (nsFlow * plan.nsDelay + ewFlow * plan.ewDelay) / totalFlow;
    } else {
        plan.overallDelay = 0.0;
    }

    return plan;
}

double estimateAverageDelay(double flow, double green, double cycle) {
    if (flow <= 0.0 || cycle <= 0.0 || green <= 0.0) {
        return 0.0;
    }

    // Approximate uniform signal delay:
    // d = 0.5*C*(1-g/C)^2 / (1 - min(1, v/c)*g/C)
    // Here v/c is approximated using a saturation flow of 1800 veh/hour.
    double greenRatio = green / cycle;
    double capacity = SATURATION_FLOW * greenRatio;
    double volumeCapacity = flow / capacity;

    // Keep the denominator positive and finite for overloaded cases.
    double denominator = 1.0 - clampValue(volumeCapacity, 0.0, 0.99) * greenRatio;
    return 0.5 * cycle * (1.0 - greenRatio) * (1.0 - greenRatio)
           / denominator;
}

double estimateQueue(double flow, double green, double cycle) {
    if (flow <= 0.0 || cycle <= 0.0) {
        return 0.0;
    }

    // Simple queue estimate: arrivals during the red part of the cycle.
    double redTime = cycle - green;
    return flow * redTime / 3600.0;
}
