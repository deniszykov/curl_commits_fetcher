#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Callback function for libcurl to write received data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// Function to get the date string for one week ago in ISO 8601 format
std::string GetOneWeekAgoDate() {
    std::time_t now = std::time(nullptr);

    std::tm one_week_ago_tm;
#ifdef __GNUC__
    localtime_r(&now, &one_week_ago_tm);
#else
    localtime_s(&one_week_ago_tm, &now);
#endif
    one_week_ago_tm.tm_mday -= 7;
    std::mktime(&one_week_ago_tm);
    
    std::ostringstream oss;
    oss << std::put_time(&one_week_ago_tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

// Function to fetch commits from GitHub API
std::vector<std::string> FetchCommitsFromGitHub(const std::string& since_date) {
    std::vector<std::string> commits;
    CURL* curl = curl_easy_init();
    std::string read_buffer;
    
    if (curl) {
        std::string url = "https://api.github.com/repos/curl/curl/commits?since=" + since_date;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl-commit-fetcher");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            try {
                json commits_json = json::parse(read_buffer);
                
                for (const auto& commit : commits_json) {
                    std::string message = commit["commit"]["message"];
                    // Take only the first line if it's a multi-line message
                    size_t newline_pos = message.find('\n');
                    if (newline_pos != std::string::npos) {
                        message = message.substr(0, newline_pos);
                    }
                    
                    std::string author = commit["commit"]["author"]["name"];
                    std::string date = commit["commit"]["author"]["date"];
                    
                    // Format the output string
                    std::ostringstream oss;
                    oss << "Message: " << message << "\n"
                        << "Author: " << author << "\n"
                        << "Date:   " << date << "\n";
                    
                    commits.push_back(oss.str());
                }
            } catch (const json::exception& e) {
                std::cerr << "JSON parsing error: " << e.what() << std::endl;
            }
        }
        
        curl_easy_cleanup(curl);
    }
    
    return commits;
}

int main() {
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Get the date from one week ago
    std::string since_date = GetOneWeekAgoDate();
    std::cout << "Fetching commits since " << since_date << "...\n\n";
    
    // Fetch commits from GitHub
    std::vector<std::string> commits = FetchCommitsFromGitHub(since_date);
    
    // Display the commits
    if (commits.empty()) {
        std::cout << "No commits found in the last week." << std::endl;
    } else {
        std::cout << "Commits from the last week (" << commits.size() << " found):\n";
        std::cout << "========================================\n";
        
        for (const auto& commit : commits) {
            std::cout << commit << "\n";
        }
    }
    
    // Cleanup libcurl
    curl_global_cleanup();
    
    return 0;
}