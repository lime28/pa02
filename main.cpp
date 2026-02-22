// Winter'24
// Instructor: Diba Mirza
// Student name: Liam Habiby (A4W6417) and Kaden Warner (A2W9Y75)
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <chrono>
using namespace std;
using namespace std::chrono;

#include "utilities.h"
#include "movies.h"

bool parseLine(string &line, string &movieName, double &movieRating);

int main(int argc, char** argv){
    // auto start = high_resolution_clock::now();

    cout << std::fixed << std::setprecision(1);

    if (argc < 2){
        cerr << "Not enough arguments provided (need at least 1 argument)." << endl;
        cerr << "Usage: " << argv[ 0 ] << " moviesFilename prefixFilename " << endl;
        exit(1);
    }

    ifstream movieFile (argv[1]);

    if (movieFile.fail()){
        cerr << "Could not open file " << argv[1];
        exit(1);
    }

    // Create an object of a STL data-structure to store all the movies
    string line, movieName;
    double movieRating;
    vector<pair<string, double>> movies;

    // Read each file and store the name and rating
    while (getline(movieFile, line) && parseLine(line, movieName, movieRating)){
        movies.push_back({movieName, movieRating});
    }

    sort(movies.begin(), movies.end());

    movieFile.close();

    if (argc == 2){
        for (auto& [name, rating] : movies) {
            cout << name << ", " << rating << "\n";
        }

        return 0;
    }

    ifstream prefixFile (argv[2]);

    if (prefixFile.fail()) {
        cerr << "Could not open file " << argv[2];
        exit(1);
    }

    vector<string> prefixes;
    while (getline (prefixFile, line)) {
        if (!line.empty()) {
            prefixes.push_back(line);
        }
    }

    vector<tuple<string, string, double>> results;

    for (string& prefix : prefixes) {
        auto lower = lower_bound(movies.begin(), movies.end(), prefix, [](const pair<string,double>& movie, const string& key) {
            return movie.first < key;
        });

        string next = prefix;
        next.back()++;

        auto last = lower_bound(movies.begin(), movies.end(), next, [](const pair<string,double>& movie, const string& key) {
            return movie.first < key;
        });

        vector<pair<string, double>> prefixArray;

        for (auto it = lower; it != last; ++it) {
            prefixArray.push_back(*it);
        }

        sort(prefixArray.begin(), prefixArray.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
            return (a.second != b.second) ? a.second > b.second : a.first < b.first;
        });

        for (auto& [name, rating] : prefixArray) {
            cout << name << ", " << rating << "\n";
        }

        if (prefixArray.size() > 0) {
            cout << "\n";
            results.push_back({prefix, prefixArray[0].first, prefixArray[0].second});
        } else {
            results.push_back({prefix, "", -1});
        }
    }

    for (auto& [prefix, name, rating] : results) {
        if (rating != -1) {
            cout << "Best movie with prefix " << prefix << " is: " << name << " with rating " << rating << "\n";
        } else {
            cout << "No movies found with prefix " << prefix << "\n";
        }
    }

    // auto end = high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> ms_double = end - start;

    // cout << "Time: " << ms_double << "\n";

    return 0;
}

/* Add your run time analysis for part 3 of the assignment here as commented block
there are n movies in the dataset.
there are m prefixes specified in a given run of your program.
at most k movies begin with each prefix.
l is the maximum number of characters in a movie name

Steps that are [negligable] have variables with a higher order of magnitude elsewhere.

Time Complexity:
Iterating through the input file and pushing to the movies vector is O(n*l) [negligable]
Sorting the movies vector is O(n*l*log(n))
Iterating through the prefix file and pushing to the prefix vector is O(m) [negligable]
For each prefix (we multiply by m):
    find the lower and upper bound of the range with given prefix using binary search in O(l*log(n))
    push all movies with prefix in the range into a vector in O(k*l) [negligable]
    sort the vector of movies with prefix in O(k*l*log(k))
    output all moves with prefix in O(k*l) [negligable]
output the result of all prefix searches in O(m*l) [negligable]

The final time complexity is O(n*l*log(n) + m(l*log(n) + k*l*log(k)))

Time checks
0 prefixes: 973ms
1000 prefixes: 1050ms
5000 prefixes: 1232ms
10000 prefixes: 1377ms
All prefixes (17577): 1845ms
Since m is the only thing variable increasing and it scales at a constant rate, the time checks out.


Space complexity:
The vector used to store all movie names and ratings is O(n*l), but this is assumed and doesn't count toward space.
The vector used to store all prefixes is O(m), but this is assumed and doesn't count toward space.
For each prefix, we store a vector of all movies with this prefix in space O(k*l).
The vector used to store the results of all prefix queries is O(m*l)

The final space complexity is O(k*l + m*l)


Explore tradeoffs between time/space complexity
I think that our algorithm had both a small time complexity and a mediocre space complexity.
I don't think it's possible for the time complexity to improve, because since the output needs to be ranges and not individual values, sorting requires logarithmic complexity.
I think the space complexity can be improved to not have the m*l because you could store the indices of an input array instead of the strings themselves
Overall, this approach prioritizes time complexity but has decent space complexity.
 */

bool parseLine(string &line, string &movieName, double &movieRating) {
    int commaIndex = line.find_last_of(",");
    movieName = line.substr(0, commaIndex);
    movieRating = stod(line.substr(commaIndex+1));
    if (movieName[0] == '\"') {
        movieName = movieName.substr(1, movieName.length() - 2);
    }
    return true;
}
