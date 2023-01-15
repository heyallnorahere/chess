/*
   Copyright 2022-2023 Nora Beda

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "pch.h"
#include "renderer.h"
#include "client.h"

namespace libchess::console {
    static int entrypoint(int argc, const char** argv) {
        renderer::init(800, 600);
        auto _client = client::create(); // todo: pass fen

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            _client->update();
            renderer::flush();

            if (_client->should_quit()) {
                break;
            }
        }

        _client.reset();
        renderer::shutdown();

        return 0;
    }
} // namespace libchess::console

int main(int argc, const char** argv) { return libchess::console::entrypoint(argc, argv); }