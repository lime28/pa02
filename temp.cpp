// Winter'24
// Instructor: Diba Mirza
// Student name: Liam Habiby (A4W6417) and Kaden Warner (A2W9Y75)
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <charconv>
#include <unordered_map>
using namespace std;
using namespace std::chrono;

#include "utilities.h"
#include "movies.h"

using Movie = pair<string_view, double>;

static inline void parseFile(string_view file, vector<Movie>& movies);
static inline bool parseLine(string_view line, vector<Movie>& movies);
static inline void appendOneDecimal(string& out, double value);

int main(int argc, char** argv){
    auto start = high_resolution_clock::now();

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string out;
    out.reserve(10000000);

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

    vector<Movie> movies;

    movieFile.seekg(0, std::ios::end);
    std::size_t size = movieFile.tellg();
    movieFile.seekg(0, std::ios::beg);

    std::string f(size, '\0');
    movieFile.read(&f[0], size);

    size_t approxLines = 0;
    for (char c : f) {
        if (c == '\n') ++approxLines;
    }
    movies.reserve(approxLines + 1);

    parseFile(f, movies);

    movieFile.close();

    if (argc == 2){
        sort(movies.begin(), movies.end(), [](const Movie& a, const Movie& b) {
            return a.first < b.first;
        });

        for (auto& [name, rating] : movies) {
            out.append(name.data(), name.size());
            out += ", ";
            appendOneDecimal(out, rating);
            out.push_back('\n');
        }

        cout.write(out.data(), static_cast<std::streamsize>(out.size()));
        return 0;
    }

    ifstream prefixFile (argv[2]);

    if (prefixFile.fail()) {
        cerr << "Could not open file " << argv[2];
        exit(1);
    }

    string line;
    vector<string> prefixes;
    while (getline (prefixFile, line)) {
        if (!line.empty()) {
            prefixes.push_back(line);
        }
    }

    vector<tuple<string_view, string_view, double>> prefixResults;
    prefixResults.reserve(prefixes.size());

    const auto byRatingDescNameAsc = [](const Movie* a, const Movie* b) {
        return (a->second != b->second) ? a->second > b->second : a->first < b->first;
    };

    const bool useHash = movies.size() > prefixes.size();

    if (useHash) {
        unordered_map<string_view, vector<const Movie*>> prefixBuckets;
        prefixBuckets.reserve(prefixes.size() * 2 + 1);

        size_t maxPrefixLen = 0;
        for (string& prefix : prefixes) {
            string_view pv(prefix);
            prefixBuckets.try_emplace(pv);
            if (pv.size() > maxPrefixLen) maxPrefixLen = pv.size();
        }

        for (const auto& movie : movies) {
            const size_t limit = min(maxPrefixLen, movie.first.size());
            for (size_t len = 1; len <= limit; ++len) {
                auto it = prefixBuckets.find(movie.first.substr(0, len));
                if (it != prefixBuckets.end()) {
                    it->second.push_back(&movie);
                }
            }
        }

        for (string& prefix : prefixes) {
            auto it = prefixBuckets.find(string_view(prefix));
            if (it == prefixBuckets.end() || it->second.empty()) {
                out += "No movies found with prefix ";
                out += prefix;
                out.push_back('\n');
                continue;
            }

            auto& matches = it->second;
            sort(matches.begin(), matches.end(), byRatingDescNameAsc);

            for (const auto* movie : matches) {
                out.append(movie->first.data(), movie->first.size());
                out += ", ";
                appendOneDecimal(out, movie->second);
                out.push_back('\n');
            }
            out.push_back('\n');

            prefixResults.emplace_back(string_view(prefix), matches[0]->first, matches[0]->second);
        }
    } else {
        sort(movies.begin(), movies.end(), [](const Movie& a, const Movie& b) {
            return a.first < b.first;
        });

        for (string& prefix : prefixes) {
            auto lower = lower_bound(movies.begin(), movies.end(), prefix, [](const Movie& movie, const string& key) {
                return movie.first < key;
            });

            string next = prefix;
            next.back()++;

            auto last = lower_bound(movies.begin(), movies.end(), next, [](const Movie& movie, const string& key) {
                return movie.first < key;
            });

            if (lower == last) {
                out += "No movies found with prefix ";
                out += prefix;
                out.push_back('\n');
                continue;
            }

            vector<const Movie*> prefixArray;
            prefixArray.reserve(static_cast<size_t>(last - lower));
            for (auto it = lower; it != last; ++it) {
                prefixArray.push_back(&(*it));
            }

            sort(prefixArray.begin(), prefixArray.end(), byRatingDescNameAsc);

            for (const auto* movie : prefixArray) {
                out.append(movie->first.data(), movie->first.size());
                out += ", ";
                appendOneDecimal(out, movie->second);
                out.push_back('\n');
            }
            out.push_back('\n');

            prefixResults.emplace_back(string_view(prefix), prefixArray[0]->first, prefixArray[0]->second);
        }
    }

    for (const auto& [prefix, name, rating] : prefixResults) {
        out += "Best movie with prefix ";
        out.append(prefix.data(), prefix.size());
        out += " is: ";
        out.append(name.data(), name.size());
        out += " with rating ";
        appendOneDecimal(out, rating);
        out.push_back('\n');
    }

    cout.write(out.data(), static_cast<std::streamsize>(out.size()));

    auto end = high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ms_double = end - start;
    cerr << "Time: " << ms_double << "\n";
    // cerr << out.size() << "\n";

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

// bool parseLine(string &line, string &movieName, double &movieRating) {
//     int commaIndex = line.find_last_of(",");
//     movieName = line.substr(0, commaIndex);
//     movieRating = stod(line.substr(commaIndex+1));
//     if (movieName[0] == '\"') {
//         movieName = movieName.substr(1, movieName.length() - 2);
//     }
//     return true;
// }

// bool parseLine(string &line, vector<pair<string, double>>& movies) {
//     int commaIndex = line.find_last_of(",");
//     movies.emplace_back(line.substr(0, commaIndex), stod(line.substr(commaIndex+1)));
//     string& movieName = movies[movies.size()-1].first;
//     if (movieName[0] == '\"') {
//         movieName = movieName.substr(1, movieName.length() - 2);
//     }
//     return true;
// }

static inline bool parseLine(std::string_view line, std::vector<Movie>& movies)
{
    const size_t comma = line.rfind(',');
    if (comma == std::string_view::npos) return false;

    std::string_view name = line.substr(0, comma);
    std::string_view rating_sv = line.substr(comma + 1);

    if (name.size() >= 2 && name.front() == '"' && name.back() == '"') {
        name = name.substr(1, name.size() - 2);
    }

    size_t i = 0;
    while (i < rating_sv.size() && (rating_sv[i] == ' ' || rating_sv[i] == '\t')) ++i;
    if (i == rating_sv.size()) return false;

    bool neg = false;
    if (rating_sv[i] == '+' || rating_sv[i] == '-') {
        neg = (rating_sv[i] == '-');
        ++i;
    }

    if (i == rating_sv.size() || rating_sv[i] < '0' || rating_sv[i] > '9') return false;

    int whole = 0;
    while (i < rating_sv.size() && rating_sv[i] >= '0' && rating_sv[i] <= '9') {
        whole = whole * 10 + (rating_sv[i] - '0');
        ++i;
    }

    int tenth = 0;
    if (i < rating_sv.size() && rating_sv[i] == '.') {
        ++i;
        if (i < rating_sv.size() && rating_sv[i] >= '0' && rating_sv[i] <= '9') {
            tenth = rating_sv[i] - '0';
            ++i;
        }
        while (i < rating_sv.size() && rating_sv[i] >= '0' && rating_sv[i] <= '9') ++i;
    }

    while (i < rating_sv.size() && (rating_sv[i] == ' ' || rating_sv[i] == '\t')) ++i;
    if (i != rating_sv.size()) return false;

    double rating = static_cast<double>(whole) + static_cast<double>(tenth) * 0.1;
    if (neg) rating = -rating;

    movies.emplace_back(name, rating);
    return true;
}

static inline void appendOneDecimal(string& out, double value) {
    if (value < 0) {
        out.push_back('-');
        value = -value;
    }

    const int scaled = static_cast<int>(value * 10.0 + 0.5);
    const int whole = scaled / 10;
    const int tenth = scaled % 10;

    char buf[24];
    auto [ptr, ec] = to_chars(buf, buf + sizeof(buf), whole);
    if (ec == std::errc{}) {
        out.append(buf, static_cast<size_t>(ptr - buf));
    } else {
        out += "0";
    }

    out.push_back('.');
    out.push_back(static_cast<char>('0' + tenth));
}

static inline void parseFile(string_view file, vector<Movie>& movies) {
    size_t start = 0;
    while (start < file.size()) {
        size_t end = file.find('\n', start);
        if (end == string_view::npos) end = file.size();

        string_view line = file.substr(start, end - start);
        parseLine(line, movies);

        start = end + 1;
    }
}
