#include <iostream>
#include "calculations.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

using namespace std;

double readNonNegative(const string& prompt) {
    double value;

    while (true) {
        cout << prompt;
        if (cin >> value && value >= 0.0) {
            return value;
        }

        cout << "Invalid input. Enter a number that is zero or greater.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double readPositive(const string& prompt) {
    double value;

    while (true) {
        cout << prompt;
        if (cin >> value && value > 0.0) {
            return value;
        }

        cout << "Invalid input. Enter a number greater than zero.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int readChoice(const string& prompt, int minimum, int maximum) {
    int value;

    while (true) {
        cout << prompt;
        if (cin >> value && value >= minimum && value <= maximum) {
            return value;
        }

        cout << "Invalid choice. Enter a number from "
             << minimum << " to " << maximum << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

TrafficInput readTrafficInput() {
    TrafficInput data{};

    cout << "\nEnter vehicle arrival rates (vehicles per hour).\n";
    data.north = readNonNegative("North: ");
    data.south = readNonNegative("South: ");
    data.east = readNonNegative("East: ");
    data.west = readNonNegative("West: ");
    data.maxDesiredWait = readPositive(
        "Desired maximum wait time in seconds (recommended 30 or more): ");

    return data;
}

void printPlan(const string& label, const TrafficInput& data,
               const SignalPlan& plan) {
    cout << "\n========== " << label << " ==========\n";
    cout << fixed << setprecision(2);
    cout << "Cycle length:             " << plan.cycleLength << " s\n";
    cout << "North/South green:        " << plan.nsGreen << " s\n";
    cout << "East/West green:          " << plan.ewGreen << " s\n";
    cout << "Estimated NS average delay:" << plan.nsDelay << " s/vehicle\n";
    cout << "Estimated EW average delay:" << plan.ewDelay << " s/vehicle\n";
    cout << "Estimated NS queue:       " << plan.nsQueue << " vehicles\n";
    cout << "Estimated EW queue:       " << plan.ewQueue << " vehicles\n";
    cout << "Estimated overall delay:  " << plan.overallDelay << " s/vehicle\n";

    if (plan.overallDelay <= data.maxDesiredWait) {
        cout << "Wait-time target:          within desired target (estimate)\n";
    } else {
        cout << "Wait-time target:          exceeds desired target (estimate)\n";
    }
}

int main() {
    cout << "========================================\n";
    cout << " P6 TRAFFIC SIGNAL TIMING OPTIMIZER\n";
    cout << " Educational four-way, two-phase model\n";
    cout << "========================================\n";

    TrafficInput traffic = readTrafficInput();

    cout << "\nChoose traffic condition:\n";
    cout << "1. Off-peak (use 60% of entered traffic)\n";
    cout << "2. Peak (use 130% of entered traffic)\n";
    cout << "3. Normal (use entered traffic)\n";
    int condition = readChoice("Choice: ", 1, 3);

    double multiplier = 1.0;
    string conditionName = "Normal";

    if (condition == 1) {
        multiplier = 0.60;
        conditionName = "Off-peak";
    } else if (condition == 2) {
        multiplier = 1.30;
        conditionName = "Peak";
    }

    TrafficInput adjusted = traffic;
    adjusted.north *= multiplier;
    adjusted.south *= multiplier;
    adjusted.east *= multiplier;
    adjusted.west *= multiplier;

    cout << "\nTraffic condition: " << conditionName
         << " (multiplier " << multiplier << ")\n";

    SignalPlan recommended = makeRecommendedPlan(adjusted);
    printPlan("RECOMMENDED PLAN", adjusted, recommended);

    cout << "\n========== CYCLE PLAN COMPARISON ==========\n";
    cout << "Cycle(s)  NS green(s)  EW green(s)  Overall delay(s/vehicle)\n";

    const double cycles[] = {60.0, 90.0, 120.0};
    SignalPlan best = makePlan(adjusted, cycles[0]);

    for (double cycle : cycles) {
        SignalPlan plan = makePlan(adjusted, cycle);
        cout << fixed << setprecision(2)
             << setw(8) << plan.cycleLength
             << setw(13) << plan.nsGreen
             << setw(13) << plan.ewGreen
             << setw(27) << plan.overallDelay << "\n";

        if (plan.overallDelay < best.overallDelay) {
            best = plan;
        }
    }

    cout << "\nBest of the three tested cycle lengths: "
         << best.cycleLength << " seconds (by estimated overall delay).\n";
    cout << "\nIMPORTANT: This is a classroom simulation, not a real-world\n";
    cout << "traffic-control recommendation. The formulas are simplified\n";
    cout << "estimates and must not be used to operate actual traffic lights.\n";

    return 0;
}
