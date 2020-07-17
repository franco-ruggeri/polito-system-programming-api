//
// Created by fruggeri on 7/15/20.
//

#include "SharedEditor.h"
#include <algorithm>
#include "NetworkServer.h"

SharedEditor::SharedEditor(NetworkServer& server) : server(server), site_counter(0) {}

std::shared_ptr<SharedEditor> SharedEditor::make_shared_editor(NetworkServer& server) {
    std::shared_ptr<SharedEditor> shared_editor(new SharedEditor(server));
    int site_id = server.connect(shared_editor);
    shared_editor->site_id = site_id;
    shared_editor->self = shared_editor;
    return shared_editor;
}

void SharedEditor::local_insert(int index, char value) {
    std::vector<int> prev_pos = index == 0 ? pos_allocator.get_begin() : symbols.at(index-1).get_position();
    std::vector<int> next_pos = index == symbols.size() ? pos_allocator.get_end() : symbols.at(index).get_position();
    std::vector<int> between_pos = pos_allocator.get_between(prev_pos, next_pos);
    Symbol symbol(value, site_id, site_counter++, between_pos);
    symbols.insert(symbols.begin()+index, symbol);
    server.send(Message(self.lock(), symbol, Message::insert));
}

void SharedEditor::local_erase(int index) {
    Symbol symbol = symbols.at(index);
    symbols.erase(symbols.begin() + index);
    server.send(Message(self.lock(), symbol, Message::erase));
}

void SharedEditor::remote_insert(const Message &message) {
    Symbol symbol = message.get_symbol();
    auto it = std::lower_bound(symbols.begin(), symbols.end(), symbol);
    symbols.insert(it, symbol);
}

void SharedEditor::remote_erase(const Message &message) {
    Symbol symbol = message.get_symbol();
    auto it = std::lower_bound(symbols.begin(), symbols.end(), symbol);
    if (it != symbols.end() && *it == symbol)
        symbols.erase(it);
}

void SharedEditor::process(const Message &message) {
    switch (message.get_type()) {
        case Message::insert:
            remote_insert(message);
            break;
        case Message::erase:
            remote_erase(message);
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
