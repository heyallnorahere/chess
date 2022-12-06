/*
   Copyright 2022 Nora Beda

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

#include <testbed.h>
#include <libchess.h>

class valid_moves : public test_theory {
public:
    virtual void add_inline_data() override {
        inline_data({  });
    }

    virtual void invoke(const std::vector<std::string>& data) override {
        // todo: run test
    }
};

DEFINE_ENTRYPOINT() {
    invoke_check<valid_moves>();
}