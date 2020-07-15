//
// Created by fruggeri on 7/15/20.
//

#include "SharedEditor.h"
#include <algorithm>
#include "NetworkServer.h"


#include <iostream>

SharedEditor::SharedEditor(NetworkServer& server) : server(server), site_counter(0) {}

std::shared_ptr<SharedEditor> SharedEditor::make_shared_editor(NetworkServer& server) {
    std::shared_ptr<SharedEditor> shared_editor(new SharedEditor(server));
    int site_id = server.connect(shared_editor);
    shared_editor->site_id = site_id;
    shared_editor->self = shared_editor;
    return shared_editor;
}

void SharedEditor::local_insert(int index, char value) {
    std::vector<int> position;
    auto n_symbols = symbols.size();

    // prepare position
    if (n_symbols == 0)             // empty => start from 0
        position.push_back(0);
    else if (index == 0)            // front => integer index (no fraction)
        position.push_back(symbols.at(index).get_position()[0] - 1);
    else if (index == n_symbols)    // back => integer index (no fraction)
        position.push_back(symbols.at(index-1).get_position()[0] + 1);
    else {                          // middle => fractional index, with as few digits as possible and maximizing margin
        std::vector<int> prev_pos = symbols.at(index-1).get_position();
        std::vector<int> next_pos = symbols.at(index).get_position();
        auto size_pos = std::min(prev_pos.size(), next_pos.size());
        bool consecutive = false;
        bool done = false;
        int middle;

        for (int i=0; i<size_pos && !done; i++) {
            // previous iteration was with difference=1 (see case 1 below)
            if (consecutive) {
                middle = (next_pos[i]+10 - prev_pos[i]) / 2 + prev_pos[i];
                done = true;
                break;
            }

            switch (next_pos[i] - prev_pos[i]) {
                case 0:
                    // example: prev_pos=0.5, next_pos=0.6, iteration=0 => put 0 and go to next
                    position.push_back(prev_pos[i]);
                    break;
                case 1:
                    // example: prev_pos=0.5, next_pos=1.1, iteration=0 => put 0, next iteration put 8 and stop
                    position.push_back(prev_pos[i]);
                    consecutive = true;
                    break;
                default:
                    // example: prev_pos=0, next_pos=4, iteration=0 => put 2 and stop
                    middle = (next_pos[i] - prev_pos[i]) / 2 + prev_pos[i];
                    done = true;
                    break;
            }
        }

        if (!done) {
            // example: prev_pos=0.4, next_pos=1, current_pos=0 => put 7 (result 0.7)
            if (prev_pos.size() > size_pos)
                middle = (10 - prev_pos[size_pos]) / 2 + prev_pos[size_pos];
            // example: prev_pos=0, next_pos=1.6, current_pos=0 => put 8 (result 0.8)
            else if (next_pos.size() > size_pos)
                middle = (next_pos[size_pos]+10) / 2;
            // example: prev_pos=0, next_pos=1, current_pos=0 => put 5 (result 0.5)
            else
                middle = 5;
        }

        // add final value that makes the position unique
        position.push_back(middle);
    }

    // insert locally and send message to server
    Symbol symbol(value, site_id, site_counter++, position);
    symbols.insert(symbols.begin()+index, symbol);
    server.send(Message(self.lock(), symbol, Message::insert));
}

void SharedEditor::local_erase(int index) {
    Symbol symbol = symbols.at(index);
    symbols.erase(symbols.begin() + index);
    server.send(Message(self.lock(), symbol, Message::erase));
}

void SharedEditor::process(const Message &message) {
    const Symbol symbol = message.get_symbol();
    std::vector<int> position = symbol.get_position();
    int site_id = symbol.get_site_id();
    int site_counter = symbol.get_site_counter();
    std::vector<Symbol>::iterator it;

    switch (message.get_type()) {
        case Message::insert:
            if (symbols.empty()) {
                symbols.push_back(symbol);
            } else {
                // search index
                it = symbols.begin();
                while (it != symbols.end()) {
                    std::vector<int> p = it->get_position();
                    if (position < p || (position == p && site_id < it->get_site_id()))
                        break;
                    it++;
                }

                // insert symbol
                symbols.insert(it, symbol);
            }
            break;
        case Message::erase:
            // search symbol
            it = std::find_if(symbols.begin(), symbols.end(), [=](const Symbol& s) {
                return s.get_position() == position && s.get_site_id() == site_id && s.get_site_counter() == site_counter;
            });

            // erase
            if (it != symbols.end())
                symbols.erase(it);
            break;
        default:
            throw std::logic_error("invalid MessageType");
    }
}

std::string SharedEditor::to_string() {
    std::string s;
    std::transform(symbols.begin(), symbols.end(), std::back_inserter(s),
            [](const Symbol& symbol) { return symbol.get_value(); });
    return s;
}
