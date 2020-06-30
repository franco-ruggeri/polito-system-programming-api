#include <iostream>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include "Message.h"
#include "DurationLogger.h"
#include "MessageStore.h"

#define SIZE 10
#define N_ADD 100
#define N_REM 50

void print_message_store_stats(const MessageStore& ms) {
    auto stats = ms.stats();
    int valid = std::get<0>(stats);
    int invalid = std::get<1>(stats);
    std::cout << "valid=" << valid << ", invalid=" << invalid << std::endl;
}

void part1_message_management() {
    /*
     * Questions 1-3
     */
    // constructor
    Message m1{10};
    std::cout << m1 << std::endl << std::endl;

    // default constructor
    Message m2;
    std::cout << m2 << std::endl << std::endl;

    // copy constructor
    Message m3{m1};
    std::cout << "original " << m1 << " @ " << (void *) m1.getData() << std::endl;
    std::cout << "copy " << m3 << " @ " << (void *) m3.getData() << std::endl << std::endl;

    // move constructor
    std::cout << "message to move " << m3 << " @ " << (void *) m3.getData() << std::endl;
    Message m4{std::move(m3)};
    if (m3.getData() == nullptr) std::cout << "moved message empty" << std::endl;
    std::cout << "message created with move " << m4 << " @ " << (void *) m4.getData() << std::endl << std::endl;

    // copy assignment operator
    Message m5{20};
    m1 = m2 = m5;
    std::cout << "original " << m5 << " @ " << (void *) m5.getData() << std::endl;
    std::cout << "copy " << m1 << " @ " << (void *) m1.getData() << std::endl;
    std::cout << "copy " << m2 << " @ " << (void *) m2.getData() << std::endl << std::endl;

    // move assignment operator
    Message m6{15};
    std::cout << "message to move " << m6 << " @ " << (void *) m6.getData() << std::endl;
    m3 = m4 = std::move(m6);
    if (m6.getData() == nullptr) std::cout << "moved message empty" << std::endl;
    std::cout << "message created with move " << m4 << " @ " << (void *) m4.getData() << std::endl;
    std::cout << "copy " << m3 << " @ " << (void *) m3.getData() << std::endl << std::endl;

    /*
     * Questions 4-9
     */
    std::cout << "create static array" << std::endl;
    Message buff1[SIZE];

    std::cout << std::endl << "create dynamic array" << std::endl;
    auto *buff2 = new Message[SIZE];

    std::cout << std::endl << "assign new message to element of array" << std::endl;
    buff1[0] = Message{10};
    buff2[0] = Message{10};

    /*
     * Questions 10-11
     */
    std::cout << std::endl << "fill static array" << std::endl;
    for (auto& i : buff1)
        i = Message{1024 * 1024};    // 1 MB
    std::cout << std::endl;

    {
        DurationLogger logger{"copy_assignment_operator"};
        for (int i=0; i<SIZE; i++)
            buff2[i] = buff1[i];
    }

    {
        DurationLogger logger{"copy_assignment_operator"};
        for (int i=0; i<SIZE; i++)
            buff2[i] = std::move(buff1[i]);
    }

    std::cout << std::endl << "delete dynamic array" << std::endl;
    delete[] buff2;
    std::cout << std::endl;
}

void part2_message_storage() {
    MessageStore ms{10};
    std::vector<long> ids;

    // add messages
    std::cout << "add messages to store" << std::endl;
    for (int i=0; i<N_ADD; i++) {
        Message m{1024*1024};
        ids.push_back(m.getId());
        ms.add(std::move(m));
    }
    print_message_store_stats(ms);

    // remove (randomly-selected) messages
    std::cout << std::endl << "remove messages from store" << std::endl;
    std::shuffle(ids.begin(), ids.end(), std::default_random_engine{});
    ids.resize(N_REM);
    for (int id : ids)
        ms.remove(id);
    print_message_store_stats(ms);

    // compact
    std::cout << std::endl << "compact store" << std::endl;
    ms.compact();
    print_message_store_stats(ms);
}

int main() {
    std::cout << "start - alive messages: " << Message::getCount() << std::endl;
    part1_message_management();
    std::cout << "part 1 done - alive messages: " << Message::getCount() << std::endl << std::endl;
    part2_message_storage();
    std::cout << "part 2 done - alive messages: " << Message::getCount() << std::endl;
    return 0;
}
