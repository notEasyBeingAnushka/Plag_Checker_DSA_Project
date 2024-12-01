#include "structures.hpp"
// -----------------------------------------------------------------------------
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and classes here
#define MOD 1000000007
#define BASE 31LL

class plagiarism_checker_t
{
    // You should NOT modify the public interface of this class.
public:
    plagiarism_checker_t(void);
    plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>>
                             __submissions);
    ~plagiarism_checker_t(void);
    void add_submission(std::shared_ptr<submission_t> __submission);

protected:
    // TODO: Add members and function signatures here
    std::queue<std::shared_ptr<submission_t>> pending_submissions;
    std::vector<std::shared_ptr<submission_t>> processed_submissions;
    std::unordered_map<long, std::chrono::steady_clock::time_point> submission_time;

    // threading related
    std::thread main_thread;
    std::mutex mtx;
    std::condition_variable cv;

    std::vector<long> power_of_base;
    std::unordered_map<long, bool> plagged;
    std::unordered_map<long, std::unordered_set<int>> hashes_of_files;

    bool stop;

    void worker_function(void); // main function that will be run by the thread

    std::pair<std::unordered_set<int>, std::vector<int>> calculate_hashes_for_token_vector(std::vector<int> __token, int length);
    void detect_plagiarism(std::shared_ptr<submission_t> &__submission);
    std::pair<int, bool> check_files(std::vector<int> &indexed_hashes, std::unordered_set<int> &hashes);
    // End TODO
};
