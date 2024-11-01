#include <vector>
#include <array>
#include <algorithm>
#include <numeric>

struct SuffixArray {
    std::vector<int> suffix_array, lcp;

    // Build suffix array and LCP array for two combined submissions
    SuffixArray(const std::vector<int>& combined, int delim) {
        int n = combined.size();
        suffix_array.resize(n);
        std::iota(suffix_array.begin(), suffix_array.end(), 0);

        // Sort suffixes using the token values (initial ranking)
        std::sort(suffix_array.begin(), suffix_array.end(), [&](int i, int j) {
            return combined[i] < combined[j];
        });

        // Construct the suffix array and LCP array
        std::vector<int> rank(n), temp_sa(n), temp_rank(n);
        for (int i = 0; i < n; ++i) rank[i] = combined[i];

        for (int len = 1; len < n; len *= 2) {
            // Sort suffixes based on first len tokens
            std::sort(suffix_array.begin(), suffix_array.end(), [&](int i, int j) {
                if (rank[i] != rank[j]) return rank[i] < rank[j];
                int ri = (i + len < n) ? rank[i + len] : -1;
                int rj = (j + len < n) ? rank[j + len] : -1;
                return ri < rj;
            });

            // Update rankings
            temp_rank[suffix_array[0]] = 0;
            for (int i = 1; i < n; ++i)
                temp_rank[suffix_array[i]] = temp_rank[suffix_array[i - 1]] +
                                             (rank[suffix_array[i - 1]] != rank[suffix_array[i]] ||
                                              rank[suffix_array[i - 1] + len] != rank[suffix_array[i] + len]);
            rank = temp_rank;
        }

        // Build LCP array for the suffixes
        lcp.resize(n, 0);
        for (int i = 0, h = 0; i < n; ++i) {
            if (rank[i] > 0) {
                int j = suffix_array[rank[i] - 1];
                while (i + h < n && j + h < n && combined[i + h] == combined[j + h]) ++h;
                lcp[rank[i]] = h;
                if (h > 0) --h;
            }
        }
    }

    // Find the longest common substring and its indices
    std::tuple<int, int, int> longest_common_substring(int delim) {
        int max_len = 0, start1 = -1, start2 = -1;
        for (int i = 1; i < lcp.size(); ++i) {
            // Ensure suffixes come from different submissions
            bool different_submissions = (suffix_array[i - 1] < delim) != (suffix_array[i] < delim);
            if (different_submissions && lcp[i] > max_len) {
                max_len = lcp[i];
                start1 = suffix_array[i - 1];
                start2 = suffix_array[i];
            }
        }
        return {max_len, start1, start2};
    }
};

// Main match_submissions function
std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2) {
    std::vector<int> combined(submission1);
    combined.push_back(-1); // delimiter to separate submissions
    combined.insert(combined.end(), submission2.begin(), submission2.end());

    int delim = submission1.size(); // delimiter index
    SuffixArray sa(combined, delim);

    // Detect short exact matches (10-20 tokens) by checking LCP array
    int total_short_match_length = 0;
    for (int i = 1; i < sa.lcp.size(); ++i) {
        bool different_submissions = (sa.suffix_array[i - 1] < delim) != (sa.suffix_array[i] < delim);
        if (different_submissions && sa.lcp[i] >= 10 && sa.lcp[i] <= 20) {
            total_short_match_length += sa.lcp[i];
        }
    }

    // Detect longest approximate match (30+ tokens, 80% similarity)
    auto [longest_len, start1, start2] = sa.longest_common_substring(delim);
    if (longest_len < 30) longest_len = 0; // Only count if it's at least 30 tokens

    // Set plagiarism flag based on the total matches found
    bool plagiarism_flag = total_short_match_length > 100 || longest_len > 40;

    // Prepare the result array
    std::array<int, 5> result;
    result[0] = plagiarism_flag ? 1 : 0;                // Flag for plagiarism
    result[1] = total_short_match_length;                // Total length of short matches
    result[2] = longest_len;                             // Length of longest approximate match
    result[3] = start1 < delim ? start1 : start1 - delim; // Start index in submission1
    result[4] = start2 < delim ? start2 : start2 - delim; // Start index in submission2

    return result;
}
