//
// Created by fruggeri on 6/24/20.
//

#pragma once

#include <iostream>

#define DEBUG true
#define COPY_SWAP false

class Message {
    long id;
    char *data;
    int size;
    static int next_id;
    static unsigned int count;                  // count instances of Message

    static char *mkMessage(int n);
#if COPY_SWAP
    friend void swap(Message& m1, Message& m2); // swap function (for copy&swap paradigm)
#endif

public:
    static const long INVALID_ID = -1;

    Message(int n);
    Message();                                  // default constructor
    Message(const Message& source);             // copy constructor
    Message(Message&& source);                  // move constructor
    ~Message();                                 // destructor

#if COPY_SWAP
    Message& operator=(Message source);         // copy assignment operator (with copy&swap paradigm)
#else
    Message& operator=(const Message& source);  // copy assignment operator (without copy&swap paradigm)
    Message& operator=(Message&& source);       // move assignment operator (without copy&swap paradigm)
#endif

    // getters
    long getId() const;
    char *getData() const;
    int getSize() const;
    static unsigned int getCount();
};

// put-to operator overloading
std::ostream& operator<<(std::ostream& out, const Message& m);
