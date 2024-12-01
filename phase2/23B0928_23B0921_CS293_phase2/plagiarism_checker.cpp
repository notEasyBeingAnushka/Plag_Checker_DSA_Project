#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class
#define TOKEN_LENGTH 15
#define EXACT_PATTERN_MATCH 75
#define NUMBER_OF_SMALL_MATCHES 10

std::vector<long> calculate_base_powers()
{
    std::vector<long> power_of_base;
    if (power_of_base.size() == 0)
    {
        power_of_base.push_back(1);
        for (unsigned int i = 1; i < 1000; i++)
        {
            power_of_base.push_back((power_of_base[i - 1] * BASE) % MOD);
        }
    }
    return power_of_base;
}

plagiarism_checker_t::plagiarism_checker_t(void)
{
    power_of_base = calculate_base_powers();
    main_thread = std::thread(&plagiarism_checker_t::worker_function, this);
    stop = false;
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions)
{
    power_of_base = calculate_base_powers();
    main_thread = std::thread(&plagiarism_checker_t::worker_function, this);
    stop = false;

    for (unsigned int i = 0; i < __submissions.size(); i++)
    {
        submission_time[__submissions[i]->id] = std::chrono::time_point<std::chrono::steady_clock>(); // setting the timestamp to zero
        processed_submissions.push_back(__submissions[i]);
        tokenizer_t tokenizer(__submissions[i]->codefile);
        hashes_of_files[__submissions[i]->id] = calculate_hashes_for_token_vector(tokenizer.get_tokens(), TOKEN_LENGTH).first;
    }
}

plagiarism_checker_t::~plagiarism_checker_t(void)
{
    std::unique_lock<std::mutex> lck(mtx); // Lock the mutex to safely update the shared `stop` flag.
    stop = true;
    lck.unlock(); // Unlock the mutex explicitly before notifying all threads.
    cv.notify_all(); // This wakes up all worker threads, allowing them to check the `stop` flag and terminate gracefully if the flag is set.

    // Ensure the main thread responsible for managing the workers is properly joined. This waits for the thread to complete its execution, preventing any undefined behavior.
    if (main_thread.joinable())
    {
        main_thread.join();
    }
}

std::pair<std::unordered_set<int>, std::vector<int>> plagiarism_checker_t::calculate_hashes_for_token_vector(std::vector<int> __token, int length)
{
    std::unordered_set<int> hashes;
    std::vector<int> hashes_with_index_current_file;
    hashes_with_index_current_file.reserve(__token.size());
    long long current_value = 0;
    for (unsigned int i = 0; i < length; i++)
        current_value = (current_value + (long long)__token[i] * power_of_base[length - i - 1]) % MOD;

    hashes.insert(current_value);
    hashes_with_index_current_file.push_back(current_value);

    for (unsigned int i = length; i < __token.size(); i++)
    {
        current_value = (current_value - (long long)__token[i - length] * power_of_base[length - 1]) % MOD;
        if (current_value < 0)
            current_value += MOD;
        current_value = ((current_value * BASE) % MOD + (long long)__token[i]) % MOD;
        hashes.insert(current_value);
        hashes_with_index_current_file.push_back(current_value);
    };
    return {hashes, hashes_with_index_current_file};
}

void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    submission_time[__submission->id] = timestamp;
    std::unique_lock<std::mutex> lck(mtx);
    pending_submissions.push(__submission);
    cv.notify_one(); // Notify one waiting worker thread that a new submission is available for processing. This wakes up a thread blocked on the condition variable `cv`.
}

void plagiarism_checker_t::worker_function(void)
{
    while (!stop)
    {
        std::shared_ptr<submission_t> submission;
        // Lock the mutex to ensure thread-safe access to shared resources.
        std::unique_lock<std::mutex> lck(mtx);

        // Wait for the condition variable to be notified or for the stop signal. The lambda checks if there are pending submissions or if the stop flag is set.
        cv.wait(lck, [this]
                { return !pending_submissions.empty() || stop; });

        if (stop)
            break;

        submission = pending_submissions.front();
        pending_submissions.pop();
        detect_plagiarism(submission);
        processed_submissions.push_back(submission);
    }
}

void plagiarism_checker_t::detect_plagiarism(std::shared_ptr<submission_t> &__submission)
{
    tokenizer_t tokenizer(__submission->codefile);
    auto [hashes_set, hashes_with_index_current_file] = calculate_hashes_for_token_vector(tokenizer.get_tokens(), TOKEN_LENGTH);
    hashes_of_files[__submission->id] = hashes_set;

    // std::cerr << "Checking for submission " << __submission->id << std::endl;

    int total_no_of_patterns_matches = 0;
    for (unsigned int i = 0; i < processed_submissions.size(); i++)
    {
        // std::cerr << "-- Checking with " << processed_submissions[i]->id << std::endl;
        // std::cerr << "---- " << hashes_of_files[processed_submissions[i]->id].size() << " " << hashes_with_index_current_file.size() << std::endl;
        
        auto result = check_files(hashes_with_index_current_file, hashes_of_files[processed_submissions[i]->id]);
        // std::cerr << "---- " << result.first << " " << result.second;

        bool isPlagged = result.second || (result.first >= NUMBER_OF_SMALL_MATCHES);
        // std::cerr << " " << isPlagged;
        
        auto duration = submission_time[__submission->id] - submission_time[processed_submissions[i]->id];
        // std::cerr << " " << std::chrono::duration_cast<std::chrono::seconds>(duration).count() << std::endl;
        
        if (std::chrono::duration_cast<std::chrono::seconds>(duration).count() <= 1)
        {
            if (isPlagged)
            {
                if (!plagged[processed_submissions[i]->id])
                {
                    if (processed_submissions[i]->student != nullptr)
                        processed_submissions[i]->student->flag_student(processed_submissions[i]);
                    if (processed_submissions[i]->professor != nullptr)
                        processed_submissions[i]->professor->flag_professor(processed_submissions[i]);
                    plagged[processed_submissions[i]->id] = true;
                }
                if (!plagged[__submission->id])
                {
                    if (__submission->student != nullptr)
                        __submission->student->flag_student(__submission);
                    if (__submission->professor != nullptr)
                        __submission->professor->flag_professor(__submission);
                    plagged[__submission->id] = true;
                }
            }
        }
        else if (isPlagged)
        {
            if (!plagged[__submission->id])
            {
                if (__submission->student != nullptr)
                    __submission->student->flag_student(__submission);
                if (__submission->professor != nullptr)
                    __submission->professor->flag_professor(__submission);
                plagged[__submission->id] = true;
            }
        }
        total_no_of_patterns_matches += result.first;
    }
    // for patchwork plagiarism
    if (total_no_of_patterns_matches >= 20)
    {
        if (!plagged[__submission->id])
        {
            if (__submission->student != nullptr)
                __submission->student->flag_student(__submission);
            if (__submission->professor != nullptr)
                __submission->professor->flag_professor(__submission);
            plagged[__submission->id] = true;
        }
    }
}
// first value = number of pattern matches of atleast length = TOKEN_LENGTH
// second value = If there is a pattern match of length EXACT_PATTERN_MATCH
std::pair<int, bool> plagiarism_checker_t::check_files(std::vector<int> &indexed_hashes, std::unordered_set<int> &hashes)
{
    std::vector<bool> matched(indexed_hashes.size() + TOKEN_LENGTH - 1, false);
    for (int i = 0; i < indexed_hashes.size(); i++)
    {
        if (hashes.find(indexed_hashes[i]) != hashes.end())
        {
            for (int j = TOKEN_LENGTH - 1; j >= 0; j--)
            {
                if (matched[i + j] == true)
                    break;
                matched[i + j] = true;
            }
        }
    }
    // calculate the size of continuous matched tokens in the file
    std::pair<int, bool> result = {0, false};
    int current_match = 0;
    for (unsigned int i = 0; i < matched.size(); i++)
    {
        if (matched[i])
        {
            current_match++;
        }
        else
        {
            result.first += current_match / TOKEN_LENGTH;
            current_match = 0;
        }
        if (current_match >= EXACT_PATTERN_MATCH)
            result.second = true;
    }
    result.first += current_match / TOKEN_LENGTH;
    return result;
}
