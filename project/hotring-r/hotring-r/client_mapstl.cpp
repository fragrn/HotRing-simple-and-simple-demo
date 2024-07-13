#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <ctime>
#include <chrono>
#include <thread>
#include <map>

using namespace std;

class MapClient {
public:
    MapClient(map<string, string> &kvStore, int hotspot_ratio)
        : kvStore(kvStore), hotspot_ratio(hotspot_ratio) {
        generateKeys(50000);
        int hotspot_size = 50000 * hotspot_ratio / 100;
        hotspot_keys.assign(keys.begin(), keys.begin() + hotspot_size);
        non_hotspot_keys.assign(keys.begin() + hotspot_size, keys.end());
    }

    void performOperations() {
        cout << "The performance of map-based Key-Value Store" << endl;
        auto start_time = chrono::steady_clock::now();
        vector<int> loads_per_minute(1, 0); // 修改为1分钟

        for (int minute = 0; minute < 1; ++minute) { // 修改为1分钟
            auto minute_start = chrono::steady_clock::now();
            int operations = 0;

            while (chrono::steady_clock::now() - minute_start < chrono::minutes(1)) {
                put();
                read();
                ++operations;
            }

            loads_per_minute[minute] = operations;
        }

        // 打印每分钟处理的负载数量
        for (int minute = 0; minute < 1; ++minute) { // 修改为1分钟
            cout << "Minute " << minute + 1 << ": " << loads_per_minute[minute] << " operations" << endl;
        }
    }

private:
    map<string, string> &kvStore;
    int hotspot_ratio;
    vector<string> keys;
    vector<string> hotspot_keys;
    vector<string> non_hotspot_keys;

    void generateKeys(int num_keys) {
        for (int i = 0; i < num_keys; ++i) {
            keys.push_back(generateRandomString(8));
        }
    }

    string generateRandomString(size_t length) {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        string str(length, 0);
        for (size_t i = 0; i < length; ++i) {
            str[i] = charset[rand() % max_index];
        }
        return str;
    }

    void put() {
        string key = chooseKey();
        string value = generateRandomString(16);
        kvStore[key] = value;
    }

    void read() {
        string key = chooseKey();
        auto result = kvStore.find(key);
        if (result != kvStore.end()) {
            // 操作成功，返回值
            // cout << result->second << endl;  // 这里可以输出结果，视需求而定
        } else {
            // cout << "Key not found" << endl;  // 这里可以输出结果，视需求而定
        }
    }

    string chooseKey() {
        if (rand() % 100 < 80) {
            return hotspot_keys[rand() % hotspot_keys.size()];
        } else {
            return non_hotspot_keys[rand() % non_hotspot_keys.size()];
        }
    }
};

int main() {
    srand(time(0));  // 设置随机种子
    map<string, string> kvStore;

    int hotspot_ratio;
    cout << "Enter the hotspot ratio (in percentage): ";
    cin >> hotspot_ratio;

    MapClient client(kvStore, hotspot_ratio);
    client.performOperations();

    return 0;
}
