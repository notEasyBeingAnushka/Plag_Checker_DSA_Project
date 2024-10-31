#include <vector>
#include <unordered_set>
#include <array>
#include <algorithm>

std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2) {
    const int SHORT_PATTERN_MIN = 10;
    const int SHORT_PATTERN_MAX = 20;
    const int LONG_PATTERN_THRESHOLD = 30;
    const double LONG_PATTERN_SIMILARITY = 0.8;

    std::array<int, 5> result = {0, 0, 0, 0, 0}; // {flag, total length of short matches, longest match length, start index 1, start index 2}
    std::unordered_set<size_t> used_indices_1, used_indices_2; // Track used indices to avoid overlap
    int total_short_match_length = 0;
    int longest_approximate_match = 0;
    int longest_start_index1 = -1;
    int longest_start_index2 = -1;

    // Utility function for hashing a vector slice
    auto hash_slice = [](const std::vector<int> &tokens, int start, int length) {
        size_t hash = 0;
        size_t prime = 31;
        for (int i = start; i < start + length; ++i) {
            hash = hash * prime + tokens[i];
        }
        return hash;
    };

    // Find exact short matches (10-20 tokens)
    for (int len = SHORT_PATTERN_MIN; len <= SHORT_PATTERN_MAX; ++len) {
        std::unordered_map<size_t, int> hash_map; // Store hashes and start indices for submission1

        // Slide through submission1 and create hashes for each window of length len
        for (int i = 0; i <= static_cast<int>(submission1.size()) - len; ++i) {
            if (used_indices_1.count(i)) continue; // Skip already matched indices
            size_t hash_val = hash_slice(submission1, i, len);
            hash_map[hash_val] = i;
        }

        // Slide through submission2 and look for matches with submission1 hashes
        for (int j = 0; j <= static_cast<int>(submission2.size()) - len; ++j) {
            if (used_indices_2.count(j)) continue;
            size_t hash_val = hash_slice(submission2, j, len);

            if (hash_map.find(hash_val) != hash_map.end()) {
                int i = hash_map[hash_val];
                bool overlap = false;

                // Check for overlaps
                for (int k = 0; k < len; ++k) {
                    if (used_indices_1.count(i + k) || used_indices_2.count(j + k)) {
                        overlap = true;
                        break;
                    }
                }

                if (!overlap) {
                    // Mark indices as used to avoid double-counting
                    for (int k = 0; k < len; ++k) {
                        used_indices_1.insert(i + k);
                        used_indices_2.insert(j + k);
                    }
                    total_short_match_length += len;
                }
            }
        }
    }

    // Find approximate matches for long patterns (30+ tokens)
    for (int len = LONG_PATTERN_THRESHOLD; len <= std::min(static_cast<int>(submission1.size()), static_cast<int>(submission2.size())); ++len) {
        for (int i = 0; i <= static_cast<int>(submission1.size()) - len; ++i) {
            for (int j = 0; j <= static_cast<int>(submission2.size()) - len; ++j) {
                int match_count = 0;

                // Calculate matching tokens for sequences starting at i and j of length len
                for (int k = 0; k < len; ++k) {
                    if (submission1[i + k] == submission2[j + k]) {
                        match_count++;
                    }
                }

                if (match_count >= len * LONG_PATTERN_SIMILARITY) {
                    // Record if this is the longest approximate match found
                    if (len > longest_approximate_match) {
                        longest_approximate_match = len;
                        longest_start_index1 = i;
                        longest_start_index2 = j;
                    }
                }
            }
        }
    }

    // Set the results based on the findings
    result[1] = total_short_match_length;
    result[2] = longest_approximate_match;
    result[3] = longest_start_index1;
    result[4] = longest_start_index2;

    // Determine plagiarism flag based on threshold
    int plagiarism_threshold = 100; // Customize threshold based on requirements
    if (total_short_match_length >= plagiarism_threshold || longest_approximate_match >= LONG_PATTERN_THRESHOLD) {
        result[0] = 1; // Flagged as plagiarized
    }

    return result;
}
