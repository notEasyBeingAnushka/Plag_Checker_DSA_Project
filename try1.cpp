#include <array>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <set>

std::array<int, 5> match_submissions(std::vector<int>& submission1, std::vector<int>& submission2) {
    std::array<int, 5> result = {0, 0, 0, 0, 0};
    const int MIN_PATTERN_LENGTH = 10;
    const int SHORT_PATTERN_MAX = 20;
    const int LONG_PATTERN_MIN = 30;
    const double SIMILARITY_THRESHOLD = 0.8;
    const int PRIME = 1000000007;
    const int BASE = 31;

    // Helper function to calculate rolling hash
    auto calculate_hash = [](const std::vector<int>& arr, int start, int len, int& hash) {
        hash = 0;
        long long power = 1;
        for (int i = 0; i < len; i++) {
            hash = (hash + (1LL * arr[start + i] * power) % PRIME) % PRIME;
            power = (power * BASE) % PRIME;
        }
        return hash;
    };

    // Helper function to calculate similarity
    auto calculate_similarity = [](const std::vector<int>& arr1, int start1, 
                                 const std::vector<int>& arr2, int start2, int len) {
        int matches = 0;
        for (int i = 0; i < len; i++) {
            if (arr1[start1 + i] == arr2[start2 + i]) matches++;
        }
        return static_cast<double>(matches) / len;
    };

    // Find short exact matches (10-20 tokens)
    std::set<std::pair<int, int>> used_positions;
    int total_match_length = 0;

    // Use rolling hash for each pattern length
    for (int len = SHORT_PATTERN_MAX; len >= MIN_PATTERN_LENGTH; len--) {
        std::unordered_map<int, std::vector<int>> hash_positions;

        // Calculate hashes for submission1
        for (int i = 0; i <= static_cast<int>(submission1.size()) - len; i++) {
            int hash = 0;
            calculate_hash(submission1, i, len, hash);
            hash_positions[hash].push_back(i);
        }

        // Check matches in submission2
        for (int i = 0; i <= static_cast<int>(submission2.size()) - len; i++) {
            int hash = 0;
            calculate_hash(submission2, i, len, hash);

            if (hash_positions.find(hash) != hash_positions.end()) {
                // Verify exact match and check for overlap
                for (int pos1 : hash_positions[hash]) {
                    bool is_exact_match = true;
                    for (int j = 0; j < len; j++) {
                        if (submission1[pos1 + j] != submission2[i + j]) {
                            is_exact_match = false;
                            break;
                        }
                    }

                    if (is_exact_match) {
                        // Check if this match overlaps with previously found matches
                        bool overlaps = false;
                        for (const auto& used : used_positions) {
                            if (pos1 < used.first + len && used.first < pos1 + len) {
                                overlaps = true;
                                break;
                            }
                        }

                        if (!overlaps) {
                            used_positions.insert({pos1, i});
                            total_match_length += len;
                            break;
                        }
                    }
                }
            }
        }
    }

    // Find longest approximate match (30+ tokens)
    int longest_match = 0;
    int best_start1 = 0;
    int best_start2 = 0;

    // Try different window sizes
    for (int len = std::min(submission1.size(), submission2.size()); 
         len >= LONG_PATTERN_MIN; len = static_cast<int>(len * 0.9)) {
        
        for (int i = 0; i <= static_cast<int>(submission1.size()) - len; i++) {
            for (int j = 0; j <= static_cast<int>(submission2.size()) - len; j++) {
                double similarity = calculate_similarity(submission1, i, submission2, j, len);
                
                if (similarity >= SIMILARITY_THRESHOLD && len > longest_match) {
                    longest_match = len;
                    best_start1 = i;
                    best_start2 = j;
                    break;  // Found a good match, try smaller window
                }
            }
            if (longest_match == len) break;
        }
        
        if (longest_match > 0) break;  // Found a match, stop searching
    }

    // Set plagiarism flag based on matches
    if (total_match_length > MIN_PATTERN_LENGTH * 3 || longest_match >= LONG_PATTERN_MIN) {
        result[0] = 1;
    }

    result[1] = total_match_length;
    result[2] = longest_match;
    result[3] = best_start1;
    result[4] = best_start2;

    return result;
}
