//
// Created by fruggeri on 7/15/20.
//

#include "NetworkServer.h"

int NetworkServer::next_editor_id = 0;

int NetworkServer::connect(std::shared_ptr<SharedEditor> shared_editor) {
    editors.push_back(shared_editor);
    return next_editor_id++;
}

void NetworkServer::disconnect(std::shared_ptr<SharedEditor> shared_editor) {
    auto it = std::find(editors.begin(), editors.end(), shared_editor);
    if (it != editors.end())
        editors.erase(it);
}

void NetworkServer::send(Message message) {
    messages.push(message);
}

void NetworkServer::dispatch_messages() {
    while (!messages.empty()) {
        Message message = messages.front();
        messages.pop();
        std::shared_ptr<SharedEditor> source = message.get_source();

        for (auto& editor : editors)
            if (editor != source)
                editor->process(message);
    }
}
