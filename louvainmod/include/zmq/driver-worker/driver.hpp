/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <iostream>
#include <zmq.h>
#include <string>
#include <cassert>
#include <functional>
#include "node.hpp"
using namespace std;

class Driver : public Node {
   public:
    Driver() {
        context = zmq_ctx_new();
        this->socket = zmq_socket(context, ZMQ_REQ);
    }

    void connect(const char* server) {
        zmq_connect(this->socket, server);
#ifdef VERBOSE
        cout << "Connect to: " << server << endl;
#endif
    }

    void connect(const char* ip, const int port) {
        stringstream ss;
        ss << ip << ':' << port;
        string server = ss.str();
        zmq_connect(this->socket, server.c_str());
#ifdef VERBOSE
        cout << "Connect to: " << server << endl;
#endif
    }

    ~Driver() {
        if (this->socket != nullptr) zmq_close(this->socket);
        if (context != nullptr) zmq_ctx_destroy(context);
    }
};
