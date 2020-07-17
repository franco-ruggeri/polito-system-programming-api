#include <iostream>
#include "NetworkServer.h"
#include "SharedEditor.h"

int main() {
    NetworkServer server;
    std::shared_ptr<SharedEditor> editor1 = SharedEditor::make_shared_editor(server);
    std::shared_ptr<SharedEditor> editor2 = SharedEditor::make_shared_editor(server);

    editor1->local_insert(0, 'c');
    editor1->local_insert(1, 'a');
    editor1->local_insert(2, 't');

    server.dispatch_messages();
    std::cout << "editor1: " << editor1->to_string() << std::endl;
    std::cout << "editor2: " << editor2->to_string() << std::endl;

    // test commutativity
    editor1->local_insert(1, 'h');
    editor2->local_erase(1);

    server.dispatch_messages();
    std::cout << "editor1: " << editor1->to_string() << std::endl;
    std::cout << "editor2: " << editor2->to_string() << std::endl;

    // test idempotency
    editor1->local_erase(0);
    editor2->local_erase(0);

    server.dispatch_messages();
    std::cout << "editor1: " << editor1->to_string() << std::endl;
    std::cout << "editor2: " << editor2->to_string() << std::endl;

    // test multiple insertion
    editor1->local_insert(1, '1');
    editor2->local_insert(1, '2');

    server.dispatch_messages();
    std::cout << "editor1: " << editor1->to_string() << std::endl;
    std::cout << "editor2: " << editor2->to_string() << std::endl;

    return 0;
}
