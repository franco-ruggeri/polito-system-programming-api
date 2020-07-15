//
// Created by fruggeri on 6/24/20.
//

#include "Message.h"
#include <string>
#include <cstring>

int Message::next_id = 0;
unsigned int Message::count = 0;

char *Message::mkMessage(int n) {
    static std::string vowels = "aeiou";
    static std::string consonants = "bcdfghlmnpqrstvz";
    char *m = new char[n+1];

    for (int i=0; i<n; i++)
        m[i] = i%2 ? vowels[rand() % vowels.size()] : consonants[rand() % consonants.size()];
    m[n] = '\0';

    return m;
}

// constructor
Message::Message(int n): id(next_id++), size(n), data(mkMessage(n)) {
    if constexpr (DEBUG) std::cout << "Message::constructor" << std::endl;
    count++;
}

// default constructor
Message::Message(): id(INVALID_ID), size(0), data(nullptr) {
    if constexpr (DEBUG) std::cout << "Message::default constructor" << std::endl;
    count++;
}

// copy constructor
Message::Message(const Message& source): id(source.id), size(source.size), data(new char[source.size+1]) {
    if constexpr (DEBUG) std::cout << "Message::copy constructor" << std::endl;
    std::memcpy(this->data, source.data, this->size+1);
    count++;
}

// destructor
Message::~Message() {
    if constexpr (DEBUG) std::cout << "Message::destructor" << std::endl;
    delete[] data;
    count--;
}

#if COPY_SWAP

// swap function (for copy&swap paradigm)
void swap(Message& m1, Message& m2) {
    std::swap(m1.id, m2.id);
    std::swap(m1.size, m2.size);
    std::swap(m1.data, m2.data);
}

// move constructor
Message::Message(Message &&source): id(-1), size(0), data(nullptr) {
    if constexpr (DEBUG) std::cout << "Message::move constructor" << std::endl;
    swap(*this, source);
    count++;
}

// copy assignment operator (with copy&swap paradigm)
Message& Message::operator=(Message source) {
    if constexpr (DEBUG) std::cout << "Message::copy assignment operator" << std::endl;
    swap(*this, source);
    return *this;
}

#else

// move constructor
Message::Message(Message &&source): id(source.id), size(source.size), data(source.data) {
    if constexpr (DEBUG) std::cout << "Message::move constructor" << std::endl;
    source.data = nullptr;
    count++;
}

// copy assignment operator (without copy&swap paradigm)
Message& Message::operator=(const Message& source) {
    if constexpr (DEBUG) std::cout << "Message::copy assignment operator" << std::endl;
    if (this != &source) {
        delete[] this->data;
        this->data = nullptr;
        this->size = source.size;
        this->id = source.id;
        this->data = new char[this->size+1];
        std::memcpy(this->data, source.data, this->size+1);
    }
    return *this;
}

// move assignment operator (without copy&swap paradigm)
Message& Message::operator=(Message&& source) {
    if constexpr (DEBUG) std::cout << "Message::move assignment operator" << std::endl;
    if (this != &source) {
        delete[] this->data;
        this->size = source.size;
        this->id = source.id;
        this->data = source.data;
        source.data = nullptr;
    }
    return *this;
}

#endif

// getters
long Message::getId() const { return id; }
char *Message::getData() const { return data; }
int Message::getSize() const { return size; }
unsigned int Message::getCount() { return count; }

// put-to operator overloading
std::ostream& operator<<(std::ostream& out, const Message& m) {
    long id = m.getId();
    std::string data = id != Message::INVALID_ID ? std::string(m.getData()).substr(0, 20) : "";
    out << "[id:" << id << "][size:" << m.getSize() << "][data:" << data << "]";
    return out;
}
