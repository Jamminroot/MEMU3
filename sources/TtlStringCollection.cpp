#include "../headers/TtlStringCollection.h"

#include <thread>
#include <utility>

TtlStringCollection::TtlStringCollection(const int pMilliseconds) {
    std::thread clearThread(&TtlStringCollection::vector_cleaner, this, pMilliseconds);
    clearThread.detach();
}

[[noreturn]] void TtlStringCollection::vector_cleaner(int pCheckPeriod) {
    using namespace std::chrono;

    for (;;) {
        // TODO: Replace with locks
        std::this_thread::sleep_for(std::chrono::milliseconds(pCheckPeriod));
        if (data.empty()) continue;
        auto index = (int) data.size() - 1;
        auto now = high_resolution_clock::now().time_since_epoch();
        auto timestamp = (unsigned int) (now.count() / 1000000);
        while (index >= 0 && !data.empty()) {
            auto item = data.at(index);
            auto stamp = item.expirationTimePoint;
            if (stamp < timestamp) {
                data.erase(data.begin() + index);
            }
            index--;
        }
    }
}

void TtlStringCollection::add(std::string &msg, int timeout) {
    using namespace std::chrono;
    auto str = TtlString();
    str.message = std::move(msg);
    str.expirationTimePoint = (unsigned int) (high_resolution_clock::now().time_since_epoch().count() / 1000000) + timeout;
    data.push_back(str);
}

std::vector<std::string> TtlStringCollection::strings() {
    std::vector<std::string> result = std::vector<std::string>();
    for (auto &ttl:data) {
        result.push_back(ttl.message);
    }
    return result;
}
