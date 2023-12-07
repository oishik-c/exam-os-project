
#include "definitions.h"
#include <bits/stdc++.h>
using namespace std;

class UserLeader
{
public:
    string UserLeadername;
    vector<pair<int, int>> attempts; // Marks and corresponding time taken

    // Constructor
    UserLeader(const string &name) : UserLeadername(name) {}

    // Function to add marks and time for a new attempt
    void addAttempt(int marks, int timeTaken)
    {
        attempts.push_back({marks, timeTaken});
    }
};

const string filename = lederboardFilePath;

// Struct to represent a leaderboard entry
struct LeaderboardEntry
{
    string UserLeadername;
    int maxMarks;
    int timeForMaxMarks;
};

// Function to read UserLeader data from file
vector<UserLeader> readUserLeaderData()
{
    vector<UserLeader> UserLeaders;
    ifstream inputFile(filename);

    if (inputFile.is_open())
    {
        string line;
        while (getline(inputFile, line))
        {
            istringstream iss(line);
            string UserLeadername;
            int mark, timeTaken;

            if (iss >> UserLeadername)
            {
                UserLeader UserLeader(UserLeadername);
                while (iss >> mark >> timeTaken)
                {
                    UserLeader.addAttempt(mark, timeTaken);
                    char c;
                    iss >> c;
                }
                UserLeaders.push_back(UserLeader);
            }
        }
        inputFile.close();
    }
    else
    {
        cerr << "Unable to open file: " << filename << endl;
    }

    return UserLeaders;
}

vector<UserLeader> UserLeaders = readUserLeaderData();
// Function to update marks and time for a specific UserLeader or add a new UserLeader
void updateMarksAndTime(const string &UserLeadername, int newMarks, int timeTaken)
{
    auto it = find_if(UserLeaders.begin(), UserLeaders.end(), [UserLeadername](const UserLeader &UserLeader)
                      { return UserLeader.UserLeadername == UserLeadername; });

    if (it != UserLeaders.end())
    {
        it->addAttempt(newMarks, timeTaken);
        cout << "Marks and time added successfully for UserLeader: " << UserLeadername << endl;
    }
    else
    {
        // UserLeader not found, add a new UserLeader
        UserLeader newUserLeader(UserLeadername);
        newUserLeader.addAttempt(newMarks, timeTaken);
        UserLeaders.push_back(newUserLeader);
        cout << "New UserLeader added successfully: " << UserLeadername << endl;
    }
}

// Function to write updated UserLeader data back to the file
void writeUserLeaderData()
{
    ofstream outputFile(filename);

    if (outputFile.is_open())
    {
        for (const UserLeader &UserLeader : UserLeaders)
        {
            outputFile << UserLeader.UserLeadername;
            for (const auto &attempt : UserLeader.attempts)
            {
                outputFile << " " << attempt.first << " " << attempt.second << " |";
            }
            outputFile << "\n";
        }
        outputFile.close();
    }
    else
    {
        cerr << "Unable to open file for writing: " << filename << endl;
    }
}

bool compareLeaderboardEntries(const LeaderboardEntry &a, const LeaderboardEntry &b)
{
    if (a.maxMarks != b.maxMarks)
    {
        return a.maxMarks > b.maxMarks;
    }
    else
    {
        return a.timeForMaxMarks < b.timeForMaxMarks;
    }
}

// Function to generate leaderboard based on max marks and time
string generateLeaderboard()
{
    // Create a vector of LeaderboardEntry for sorting
    vector<LeaderboardEntry> leaderboardData;
    for (const UserLeader &UserLeader : UserLeaders)
    {
        int maxMarks = 0;
        int timeForMaxMarks = 0;

        for (const auto &attempt : UserLeader.attempts)
        {
            if (attempt.first > maxMarks)
            {
                maxMarks = attempt.first;
                timeForMaxMarks = attempt.second;
            }
        }

        leaderboardData.push_back({UserLeader.UserLeadername, maxMarks, timeForMaxMarks});
    }

    // Sort the vector based on max marks in descending order and time for max marks in ascending order
    sort(leaderboardData.begin(), leaderboardData.end(), compareLeaderboardEntries);

    // Display the sorted leaderboard
    string leaderboard = "\nLeaderboard:\n";
    for (size_t i = 0; i < leaderboardData.size(); ++i)
    {
        const auto &entry = leaderboardData[i];
        leaderboard += to_string(i + 1) + ". " + entry.UserLeadername + ": " + to_string(entry.maxMarks) + " | " + to_string(entry.timeForMaxMarks) + "s\n";
    }
    return leaderboard;
}