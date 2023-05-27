#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <map>

using namespace std;

// Schedule struct
struct Schedule {
    string placeName;
    string salonName;
    string startTime;
    string endTime;
    int startMinute;
    int endMinute;
    int capacity;
    int totalCapacity;
};

// Asset Struct
struct Asset {
    string assetName;
    float price;
    float value;
};

// Availability Struct
struct Availability {
    string placeName;
    string startDate;
    string endDate;
};

// Event struct
struct Event {
    string placeName;
    string startTime;
    string endTime;
    int startDay;
    int endDay;
    int value;
};

// Helper function for clock conversion
int convertClock2Minute(string clockString){
    size_t dotPos = clockString.find(":");

    std::string hourString = clockString.substr(0, dotPos);
    std::string minuteString = clockString.substr(dotPos + 1);
    
    int hour = std::stoi(hourString);
    int minute = std::stoi(minuteString);

    return hour * 60 + minute;
}

// Helper function for date conversion
int convertDate2Day(string dateString){  
    size_t dotPos = dateString.find(".");

    std::string dayString = dateString.substr(0, dotPos);
    std::string monthString = dateString.substr(dotPos + 1);
    
    int day = std::stoi(dayString);
    int month = std::stoi(monthString);
    if (month == 5)
        return day + 120;
    else if (month == 6)
        return day + 151;
    else
        cout << "Unexpected Month";
    return 0;
}

// Helper function for date conversion
string convertDate(string dateString){
    size_t dotPos = dateString.find(".");

    std::string dayString = dateString.substr(0, dotPos);
    std::string monthString = dateString.substr(dotPos + 1);
    
    int day = std::stoi(dayString);
    int month = std::stoi(monthString);
    if (month == 5)
        return "May " + to_string(day);
    else if (month == 6)
        return "June " + to_string(day);
    else
        cout << "Unexpected Month";
    return 0;
}

// Knapsack solution
vector<Asset> knapsack(const vector<Asset>& assets, float budget, float& totalValue) {
    int totalPrice = 0;
    int n = assets.size();
    // Fill DP table
    vector<vector<float>> dp(n + 1, vector<float>(int(budget) + 1, 0));
    for (int i = 0; i <= n; i++) {
        for (int w = 0; w <= budget; w++) {
            if (i == 0 || w == 0)
                dp[i][w] = 0;
            else if (assets[i - 1].price <= w)
                dp[i][w] = max(dp[i - 1][w], assets[i - 1].value + dp[i - 1][w - int(assets[i - 1].price)]);
            else
                dp[i][w] = dp[i - 1][w];
        }
    }

    float res = dp[n][int(budget)];
    float w = budget;
    vector<Asset> assetList;

    for (int i = n; i > 0 && res > 0; i--) {
        if (res == dp[i - 1][int(w)])
            continue;
        else {
            if ((w - assets[i - 1].price) < 0 )
                continue;
            assetList.push_back(assets[i - 1]);
            totalValue += assets[i - 1].value;
            res -= assets[i - 1].value;
            w -= assets[i - 1].price;
            totalPrice += assets[i - 1].price;
        }
    }
    return assetList;
}
// Checker function
void writeKnapsack(vector<Asset> assetList, string caseNo, float totalValue){
    string baseDir = "outputs/case" + caseNo;
    ofstream output(baseDir + "/upgrade_list.txt");
    output << "Total Value --> " << totalValue << endl;
    for (auto asset: assetList){
        output << asset.assetName << endl;
    }
}

// For tour calculation convert schedules to events
vector<Event> convertSchedulesToEvents(const map<string, vector<Schedule>>& optimalSchedules, const vector<Availability>& availabilities) {
    vector<Event> events;   

    // Create a map from place
    map<string, vector<Availability> > availabilityMap;
    for (const auto& availability : availabilities) {
        availabilityMap[availability.placeName].push_back(availability);
    }

    for (const auto& optimalSchedule : optimalSchedules) {
        const string& placeName = optimalSchedule.first;
        const vector<Schedule>& schedules = optimalSchedule.second;

        // Sum up the capacities of the schedules
        int totalCapacity = 0;
        for (const auto& schedule : schedules) {
            totalCapacity += schedule.capacity;
        }
        
        for (auto availability: availabilityMap[placeName]){
            // Create the event objects
            int startDay = convertDate2Day(availability.startDate);
            int endDay = convertDate2Day(availability.endDate);
            Event event = {placeName, availability.startDate, availability.endDate, startDay,  endDay, (endDay - startDay) * totalCapacity};
            events.push_back(event);
        }
    }

    return events;
}

// Check conflict for Tour case
int latestNonConflictTour(const vector<Event>& events, int index) {
    for (int i = index - 1; i >= 0; i--) {
        if (events[i].endDay <= events[index].startDay) {
            return i;
        }
    }
    return -1;  // Returns -1 if no non-conflicting event is found
}

// Weighted interval scheduling tour case
vector<Event> findBestTour(const vector<Event>& events, float& totalRevenue) {
    // Sort events by end time
    vector<Event> sortedEvents = events;
    sort(sortedEvents.begin(), sortedEvents.end(),
              [](const Event& a, const Event& b) { return a.endDay < b.endDay; });

    // Initialize DP table
    vector<int> dp(sortedEvents.size());
    dp[0] = sortedEvents[0].value;

    // Fill DP table
    for (int i = 1; i < sortedEvents.size(); i++) {
        int include = sortedEvents[i].value;
        int l = latestNonConflictTour(sortedEvents, i);
        if (l != -1) {
            include += dp[l];
        }

        dp[i] = max(include, dp[i - 1]);
    }

    // Find the tour with maximum value
    int maxValue = dp.back();
    vector<Event> maxTour;
    for (int i = dp.size() - 1; i >= 0; i--) {
        if (dp[i] == maxValue && (i == 0 || dp[i - 1] != maxValue)) {
            maxTour.push_back(sortedEvents[i]);
            totalRevenue += sortedEvents[i].value;
            maxValue -= sortedEvents[i].value;
            if (maxValue == 0) {
                break;
            }
        }
    }

    // Sort maxTour by start time
    sort(maxTour.begin(), maxTour.end(),
              [](const Event& a, const Event& b) { return a.startDay < b.startDay; });

    return maxTour;
}

// Helper function for checking bestTour
void writeBestTour(vector<Event> bestTour, float totalRevenue, string caseNo){
    string baseDir = "outputs/case" + caseNo;
    ofstream output(baseDir + "/best_tour.txt");
    output << "Total Revenue --> " << totalRevenue << endl;
    for (auto tour: bestTour){
        output << tour.placeName << "\t" << convertDate(tour.startTime) << "\t" << convertDate(tour.endTime) << endl; 
    }
    cout << endl;
}

// Check conflict for places case
int latestNonConflict(const vector<Schedule>& schedules, int index) {
    for (int i = index - 1; i >= 0; i--) {
        if (schedules[i].endTime <= schedules[index].startTime) {
            return i;
        }
    }
    return -1;  // Returns -1 if no non-conflicting schedule is found
}
// Weighted interval scheduling place case
map<string, vector<Schedule>> findBestScheduleForEachPlace(const vector<Schedule>& schedules) {
    map<string, vector<Schedule>> placeSchedules;
    map<string, vector<Schedule>> bestSchedules;

    // Group schedules by place
    for (const auto& schedule : schedules) {
        placeSchedules[schedule.placeName].push_back(schedule);
    }

    // Find the best schedule for each place
    for (auto& place : placeSchedules) {
        int total = 0;
        // Sort schedules by end time
        sort(place.second.begin(), place.second.end(),
                  [](const Schedule& a, const Schedule& b) { return a.endMinute < b.endMinute; });

        // Initialize DP table
        vector<int> dp(place.second.size());
        dp[0] = place.second[0].capacity;

        // Fill DP table
        for (int i = 1; i < place.second.size(); i++) {
            int include = place.second[i].capacity;
            int l = latestNonConflict(place.second, i);
            if (l != -1) {
                include += dp[l];
            }

            dp[i] = max(include, dp[i - 1]);
        }

        // Find the schedule with maximum capacity
        int maxCapacity = dp.back();
        vector<Schedule> maxSchedule;
        for (int i = dp.size() - 1; i >= 0; i--) {
            if (dp[i] == maxCapacity && (i == 0 || dp[i - 1] != maxCapacity)) {
                total += place.second[i].capacity;
                maxSchedule.push_back(place.second[i]);
                maxCapacity -= place.second[i].capacity;
                if (maxCapacity == 0) {
                    break;
                }
            }
        }

        // Sort maxSchedule by start time
        sort(maxSchedule.begin(), maxSchedule.end(),
                  [](const Schedule& a, const Schedule& b) { return a.startMinute < b.startMinute; });

        maxSchedule[0].totalCapacity = total;
        bestSchedules[place.first] = maxSchedule;
    }

    return bestSchedules;
}
// Helper function for checking optimalSchedules
void writeOptimalSchedules(map<string, vector<Schedule>> optimalSchedules, string caseNo){
    string baseDir = "outputs/case" + caseNo;
    ofstream output(baseDir + "/best_for_eachplace.txt");
    string lastPlaceName = " ";
    int counter = 0;
    for (auto optimalSchedule: optimalSchedules){
        if (counter != 0)
            output << endl;
        counter += 1;
        output << optimalSchedule.first << " --> " << optimalSchedules[optimalSchedule.first][0].totalCapacity << endl;
        for (auto schedule: optimalSchedule.second){
            output << schedule.placeName << "\t" << schedule.salonName << "\t" << schedule.startTime << "\t" << schedule.endTime << endl;
        }
    }
    cout << endl;
}

// Read schedule and capacity
vector<Schedule> readScheduleAndCapacity(string baseInputPath) {
    vector<Schedule> schedules;
    map<pair<string, string>, int> capacities; 

    ifstream scheduleFile(baseInputPath + "/daily_schedule.txt");
    ifstream capacityFile(baseInputPath + "/capacity.txt");

    string line;

    // Skip the header line
    getline(scheduleFile, line);
    getline(capacityFile, line);

    string placeName, salonName, startTime, endTime;
    int capacity;

    // Read the capacities
    while (getline(capacityFile, line)) {
        istringstream capacityStream(line);
        capacityStream >> placeName >> salonName >> capacity;
        capacities[{placeName, salonName}] = capacity;
    }

    // Read the schedules and attach capacities
    while (getline(scheduleFile, line)) {
        istringstream scheduleStream(line);
        scheduleStream >> placeName >> salonName >> startTime >> endTime;

        auto it = capacities.find({placeName, salonName});
        if (it != capacities.end()) {
            capacity = it->second;
            Schedule schedule = {placeName, salonName, startTime, endTime, convertClock2Minute(startTime), convertClock2Minute(endTime), capacity, 0};
            schedules.push_back(schedule);
        }
    }

    return schedules;
}
void checkScheduleReadings(vector <Schedule> schedules){
    for (auto schedule: schedules){
        cout << schedule.placeName << "\t" << schedule.salonName << "\t" << schedule.capacity << "\t" << schedule.startTime << "\t" << schedule.endTime << endl; 
    }
}

// Read asset
vector<Asset> readAssets(string baseInputPath) {
    vector<Asset> assets;
    
    ifstream assetFile(baseInputPath + "/assets.txt");

    string line;

    // Skip the header line
    getline(assetFile, line);

    string assetName;
    float price, value;

    while (getline(assetFile, line)) {
        istringstream assetStream(line);
        assetStream >> assetName >> price >> value;

        Asset asset = {assetName, price, value};
        assets.push_back(asset);
    }

    return assets;
}
void checkAssetReadings(vector <Asset> assets){
    for (auto asset: assets){
        cout << asset.assetName << "\t" << asset.price << "\t" << asset.value << endl;
    }
}

// Read availability
vector<Availability> readAvailability(string baseInputPath) {
    vector<Availability> availabilities;
    
    ifstream availabilityFile(baseInputPath + "/availability_intervals.txt");

    string line;

    // Skip the header line
    getline(availabilityFile, line);

    string placeName, startDate, endDate;

    while (getline(availabilityFile, line)) {
        istringstream availabilityStream(line);
        availabilityStream >> placeName >> startDate >> endDate;

        Availability availability = {placeName, startDate, endDate};
        availabilities.push_back(availability);
    }

    return availabilities;
}
void writeAvailabilityReadings(vector <Availability> availabilities){
    for (auto availability: availabilities){
        cout << availability.placeName << "\t" << availability.startDate << "\t" << availability.endDate << endl;
    }
}


int main(int argc, char *argv[]) {
    cout << "!!!!!!!! PLEASE CREATE FOLDER THAT NAMED 'outputs/case{no}/ !!!!!!!!'";
    if (argc != 2) {
        cerr << "WRONG USAGE" << endl;
        return 1;
    }
    string caseNo = argv[1];
    string baseInputPath = "inputs/case_" + caseNo;

    vector<Schedule> schedules = readScheduleAndCapacity(baseInputPath);
    // checkScheduleReadings(schedules);

    vector<Asset> assets = readAssets(baseInputPath);
    // checkAssetReadings(assets);

    vector<Availability> availabilities = readAvailability(baseInputPath);
    // checkAvailabilityReadings(availabilities);

    map<string, vector<Schedule>> optimalSchedules = findBestScheduleForEachPlace(schedules);
    writeOptimalSchedules(optimalSchedules, caseNo);
    float totalRevenue = 0, totalValue = 0;
    vector<Event> events = convertSchedulesToEvents(optimalSchedules, availabilities);
    vector<Event> bestTour = findBestTour(events, totalRevenue);
    writeBestTour(bestTour, totalRevenue, caseNo);

    vector<Asset> assetList = knapsack(assets, totalRevenue, totalValue);
    writeKnapsack(assetList, caseNo, totalValue);

    return 0;
}
