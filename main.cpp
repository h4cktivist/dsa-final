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
        static bool getRandom64(uint32_t &random) {
            return _rdrand32_step(&random);
        }
};


template <typename Generator>
void measureTime(Generator generator, int n, const std::string &funcName, std::ofstream &csvFile) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        generator.next();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    csvFile << n << "," << funcName << "," << duration.count() << "\n";
    std::cout << funcName << " with n = " << n << " took " << duration.count() << " nanoseconds.\n";
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
    uint32_t rand32;
    for (int i = 0; i < n; i++) {
        RDRand::getRandom64(rand32);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    csvFile << n << "," << "RDRand" << "," << duration.count() << "\n";
    std::cout << "RDRand" << " with n = " << n << " took " << " nanoseconds.\n";
}

void writeSampleRDRand(int n, const std::string &fileName) {
    std::ofstream sampleFile(fileName);
    uint32_t rand32;
    for (int i = 0; i < n; i++) {
        RDRand::getRandom64(rand32);
        sampleFile << rand32 << std::endl;
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

    std::vector<int> N = {static_cast<int>(1e2), static_cast<int>(1e3), static_cast<int>(1e4)};
    LCG lcg(12345);
    MersenneTwister mt(12345);
    Xorshift xorshift(12345);

    for (int n : N) {
        writeSample(lcg, static_cast<int>(1e5), "lcg_sample.txt");
        writeSample(mt, static_cast<int>(1e5), "mt_sample.txt");
        writeSample(xorshift, static_cast<int>(1e5), "xorshift_sample.txt");
        writeSampleRDRand(static_cast<int>(1e5), "rdrand_sample.txt");

        measureTime(lcg, n, "LCG", csvFile);
        measureTime(mt, n, "MersenneTwister", csvFile);
        measureTime(xorshift, n, "Xorshift", csvFile);
        measureTimeRDRand(n, csvFile);
        std::cout << "-----------" << std::endl;
    }

    csvFile.close();

    return 0;
}
