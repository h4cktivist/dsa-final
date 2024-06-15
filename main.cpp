#include <iostream>
#include <cstdint>
#include <random>
#include <chrono>
#include <vector>
#include <fstream>
#include <immintrin.h>


class LCG {
    private:
        uint64_t seed;
        static const uint64_t a = 6364136223846793005ULL;
        static const uint64_t c = 1ULL;
        static const uint64_t m = 1ULL << 32;

    public:
        explicit LCG(uint64_t seed) : seed(seed) {}

        uint32_t next() {
            seed = (a * seed + c) % m;
            return static_cast<uint32_t>(seed);
        }
};

class MersenneTwister {
    private:
        std::mt19937 mt;

    public:
        explicit MersenneTwister(uint32_t seed) : mt(seed) {}

        uint32_t next() {
            return mt();
        }
};

class Xorshift {
    private:
        uint32_t state;

    public:
        explicit Xorshift(uint32_t seed) : state(seed) {}

        uint32_t next() {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return state;
        }
};

class RDRand {
    public:
        static bool getRandom64(uint64_t &random) {
            return _rdrand64_step(&random);
        }
};


template <typename Generator>
void measureTime(Generator generator, int n, const std::string &funcName, std::ofstream &csvFile) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        generator.next();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    csvFile << n << "," << funcName << "," << duration.count() << "\n";
    std::cout << funcName << " with n = " << n << " took " << duration.count() << " seconds.\n";
}

template <typename Generator>
void writeSample(Generator generator, int n, const std::string &fileName) {
    std::ofstream sampleFile(fileName);
    for (int i = 0; i < n; i++) {
        sampleFile << generator.next() << std::endl;
    }
    sampleFile.close();
}

void measureTimeRDRand(int n, std::ofstream &csvFile) {
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t rand64;
    for (int i = 0; i < n; i++) {
        RDRand::getRandom64(rand64);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    csvFile << n << "," << "RDRand" << "," << duration.count() << "\n";
    std::cout << "RDRand" << " with n = " << n << " took " << duration.count() << " seconds.\n";
}

void writeSampleRDRand(int n, const std::string &fileName) {
    std::ofstream sampleFile(fileName);
    uint64_t rand64;
    for (int i = 0; i < n; i++) {
        RDRand::getRandom64(rand64);
        sampleFile << rand64 << std::endl;
    }
    sampleFile.close();
}


int main() {
    std::ofstream csvFile("timing_results.csv");
    if (!csvFile.is_open()) {
        std::cerr << "Failed to open files for writing." << std::endl;
        return 1;
    }
    csvFile << "n,Function,Time\n";

    std::vector<int> N = {static_cast<int>(1e4), static_cast<int>(1e5), static_cast<int>(1e6),
                          static_cast<int>(1e7), static_cast<int>(1e8), static_cast<int>(1e9)};
    LCG lcg(12345);
    MersenneTwister mt(12345);
    Xorshift xorshift(12345);

    for (int n : N) {
        if (n == static_cast<int>(1e5)) {
            writeSample(lcg, n, "lcg_sample.txt");
            writeSample(mt, n, "mt_sample.txt");
            writeSample(xorshift, n, "xorshift_sample.txt");
            writeSampleRDRand(n, "rdrand_sample.txt");
        }

        measureTime(lcg, n, "LCG", csvFile);
        measureTime(mt, n, "MersenneTwister", csvFile);
        measureTime(xorshift, n, "Xorshift", csvFile);
        measureTimeRDRand(n, csvFile);
        std::cout << "-----------" << std::endl;
    }

    csvFile.close();

    return 0;
}
