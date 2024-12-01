#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
// -----------------------------------------------------------------------------
#include <unordered_set>
#include <algorithm>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

// global variables
long long int mod = 1000000007;
long long int precompute_base_power_9()
{
    long long int base_power_9 = 1;
    for (int i = 0; i < 9; i++)
    {
        base_power_9 = (base_power_9 * 31) % mod;
    }
    return base_power_9;
}
long long int base_power_9 = precompute_base_power_9();

// polynomial hash function (very less chances of collision)
long long int hash_function(std::vector<int> &submission, int start, int token_length = 10)
{
    long long int base = 1;
    long long int hash = 0;
    for (int i = start + token_length - 1; i >= start; i--)
    {
        hash = (hash + (submission[i] * base) % mod) % mod;
        base = (base * 31LL) % mod;
    }
    return hash;
}

// function for rolling hash i.e updating hash as we move forward
long long int rolling_hash(long long int old_hash, int old_token, int new_token)
{
    long long int new_hash = (old_hash - (old_token * base_power_9) % mod) % mod;
    new_hash = (new_hash + mod) % mod;
    new_hash = (new_hash * 31 + new_token) % mod;
    return new_hash;
}

// function to calculate hashes of all tokens in a submission
std::unordered_multiset<long long int> calculate_hashes(std::vector<int> &submission, int token_length = 10)
{
    std::unordered_multiset<long long int> hashes;
    long long int hash = hash_function(submission, 0);
    hashes.insert(hash);
    for (int i = 1; i <= submission.size() - token_length; i++)
    {
        hash = rolling_hash(hash, submission[i - 1], submission[i + token_length - 1]);
        hashes.insert(hash);
    }
    return hashes;
}

// function for finding exact matches (hashes of tokens are same)
int find_exact_matches(std::vector<int> &submission1, std::vector<int> &submission2)
{
    int token_length = 10;
    int total_length_of_tokens = 0;

    int p = submission1.size();
    int q = submission2.size();

    std::vector<bool> matched2(q, false);
    auto hashes1 = calculate_hashes(submission1);
    // finding matches in submission2 for tokens of submission1
    for (int i = 0; i <= q - token_length; i++)
    {
        auto hash2 = hash_function(submission2, i);
        auto it = hashes1.find(hash2);
        if (it != hashes1.end())
        {
            hashes1.erase(it); // to avoid overlapping matches
            for (int j = 0; j < token_length; j++)
            {
                matched2[i + j] = true;
            }
        }
    }

    // chechking in reverse direction also to avoid overlapping matches
    std::vector<bool> matched1(p, false);
    auto hashes2 = calculate_hashes(submission2);
    for (int i = 0; i <= p - token_length; i++)
    {
        auto hash1 = hash_function(submission1, i);
        auto it = hashes2.find(hash1);
        if (it != hashes2.end())
        {
            hashes2.erase(it);
            for (int j = 0; j < token_length; j++)
            {
                matched1[i + j] = true;
            }
        }
    }

    int a = count(matched2.begin(), matched2.end(), true);
    int b = count(matched1.begin(), matched1.end(), true);

    total_length_of_tokens = std::min(a, b);
    return total_length_of_tokens;
}

// function to calculate edit distance dp
std::vector<std::vector<int>> edit_distance(std::vector<int> &submission1, std::vector<int> &submission2)
{
    int p = submission1.size();
    int q = submission2.size();
    std::vector<std::vector<int>> dp(p + 1, std::vector<int>(q + 1, 0));
    for (int i = 0; i <= p; i++)
    {
        dp[i][0] = i;
    }
    for (int j = 0; j <= q; j++)
    {
        dp[0][j] = j;
    }
    for (int i = 1; i <= p; i++)
    {
        for (int j = 1; j <= q; j++)
        {
            if (submission1[i - 1] == submission2[j - 1])
            {
                dp[i][j] = dp[i - 1][j - 1];
            }
            else
            {
                dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]}); // handling insert, delete and replace operations
            }
        }
    }
    return dp;
}

// function to find approximate matches (edit distance < 0.2 * length of longest common subsequence)
int find_approximate_matches(std::vector<int> &submission1, std::vector<int> &submission2, int &start_index_submission1, int &start_index_submission2)
{
    int longest_pattern_match = 0;
    int p = submission1.size();
    int q = submission2.size();
    auto dp = edit_distance(submission1, submission2);
    double threshold = 0.8;
    for (int len = std::min(p,q); len >= 30; len--)
    {
        for (int i = 0; i + len <= p; i++)
        {
            for (int j = 0; j + len <= q; j++)
            {
                if (dp[i + len][j + len] - dp[i][j] < static_cast<int>((1 - threshold) * len))
                {
                    longest_pattern_match = len;
                    start_index_submission1 = i;
                    start_index_submission2 = j;
                    return longest_pattern_match;
                }
            }
        }
    }
    return 0;
}

std::array<int, 5> match_submissions(std::vector<int> &submission1,
                                     std::vector<int> &submission2)
{
    // TODO: Write your code here
    int flag = 0;
    int total_length_of_tokens = 0;
    int longest_pattern_match = 0;
    int start_index_submission1 = -1;
    int start_index_submission2 = -1;

    // finding exact matches
    total_length_of_tokens = find_exact_matches(submission1, submission2);

    // finding approximate matches
    longest_pattern_match = find_approximate_matches(submission1, submission2, start_index_submission1, start_index_submission2);

    int threshold_exact = 0.5 * std::min(submission1.size(), submission2.size());  // if more than 50% tokens are same exactly
    int threshold_approx = 0.7 * std::min(submission1.size(), submission2.size()); // if more than 70% tokens are same approximately

    if (total_length_of_tokens >= threshold_exact || longest_pattern_match >= threshold_approx)
    {
        flag = 1;
    }

    std::array<int, 5> result = {flag, total_length_of_tokens, longest_pattern_match, start_index_submission1, start_index_submission2};
    return result; // dummy return
    // End TODO
}
