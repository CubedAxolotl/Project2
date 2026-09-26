#undef NDEBUG 
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "core/conversation.h"
#include "core/sentinel_scanner.h"
#include "model/scripted_client.h"
#include "model/replay_client.h"
#include "harness/harness.h"

const std::string SENTINEL = "<|end_conversation|>";


bool contains(const std::string& text, const std::string& part) {
    size_t position = text.find(part);

    if (position == std::string::npos) {
        return false;
    }

    return true;
}

// Writes text into a file.
void write_file(const std::string& path, const std::string& text) {
    std::ofstream file(path);
    file << text;
    file.close();
}

// Feeds text to a new scanner in pieces of `chunk` characters. Returns all safe text (plus flush), and sets `found` if the sentinel was found.
std::string scan(const std::string& text, std::size_t chunk, bool& found) {
    SentinelScanner scanner(SENTINEL);
    std::string output = "";

    found = false;

    for (std::size_t i = 0; i < text.size(); i += chunk) {
        std::string piece = text.substr(i, chunk);

        SentinelScanner::Out out = scanner.feed(piece);
        output += out.safe_text;

        if (out.sentinel_found) {
            found = true;
            return output;
        }
    }

    SentinelScanner::Out leftover = scanner.flush();
    output += leftover.safe_text;

    return output;
}

// A fake user who types the given lines, then EOF's.
class FakeInput : public InputSource {
public:
    std::string lines[10];
    int count = 0;
    int next = 0;
    bool eof = false;

    void add_line(const std::string& line) {
        lines[count] = line;
        count = count + 1;
    }

    std::string read_line() {
        if (next < count) {
            std::string line = lines[next];
            next = next + 1;
            return line;
        }

        eof = true;

        return "";
    }

    bool is_eof() const {
        return eof;
    }
};

// Collects everything the program would print.
class FakeOutput : public OutputSink {
public:
    std::string text = "";

    void write(std::string_view t) {
        text.append(t);
    }
};

// Writes a script file, runs the harness on it with 5 lines of user input, and returns why it stopped. The printed output goes into `printed`.
StopReason run_script(const std::string& script, int max_turns, std::string& printed) {
    std::string path = "test.script";
    write_file(path, script);

    HarnessConfig cfg;
    cfg.max_turns = max_turns;

    std::unique_ptr<ScriptedModelClient> client = std::make_unique<ScriptedModelClient>(path);
    Harness harness(std::move(client), cfg);

    FakeInput in;
    for (int i = 0; i < 5; i++) {
        in.add_line("hello");
    }

    FakeOutput out;

    StopReason reason = harness.run(in, out);
    printed = out.text;

    std::remove(path.c_str());

    return reason;
}

void test1_empty() {
    Conversation conv;

    assert(conv.size() == 0);

    assert(conv.begin() == conv.end());

    int count = 0;
    for (const Message& m : conv) {
        (void)m;
        count = count + 1;
    }

    assert(count == 0);  // a range-for loop runs zero times
}

void test2_system_first() {
    Conversation conv;
    conv.append(Message(Role::System, "Be concise."));
    for (int i = 0; i < 50; i++) {
        conv.append(Message(Role::User, "hi"));
    }

    Conversation copy = conv;
    Conversation moved = std::move(copy);

    assert(conv.at(0).role() == Role::System);   // after many reallocations
    assert(moved.at(0).role() == Role::System);  // after copy and move
    assert(moved.at(0).content() == "Be concise.");
}

void test3_copy() {
    Conversation a;
    a.append(Message(Role::User, "hello"));

    Conversation b(a);                 // copy constructor

    assert(b.begin() != a.begin());    // separate memory
    assert(b.at(0).content() == "hello");
    b.append(Message(Role::User, "more"));
    assert(a.size() == 1);             // original unchanged

    Conversation c;
    c = a;                             // copy assignment
    assert(c.begin() != a.begin());
    assert(c.at(0).content() == "hello");
    Conversation& same = c;
    c = same;                          // self-assignment is safe

    assert(c.size() == 1);
}

void test4_move() {
    Conversation a;
    a.append(Message(Role::User, "hello"));
    const Message* buffer = a.begin();

    Conversation b(std::move(a));      // move constructor
    assert(b.begin() == buffer);       // same memory, nothing copied
    assert(a.size() == 0);             // source is empty
    assert(a.begin() == nullptr);      // source pointer is cleared

    Conversation c;
    c = std::move(b);                  // move assignment
    assert(c.begin() == buffer);
    assert(b.size() == 0);
    assert(b.begin() == nullptr);
}

// Tests growth of append. capacity_ is private, so we keep our own copy of what the capacity should be. begin() changes exactly when the array grows.
void test5_growth() {
    Conversation conv;
    std::size_t expected_capacity = 0;

    for (std::size_t i = 0; i < 100; i++) {
        // The array should grow when it is full, doubling (or going from 0 to 1).
        bool should_grow = false;
        if (conv.size() == expected_capacity) {
            should_grow = true;

            if (expected_capacity == 0) {
                expected_capacity = 1;
            } else {
                expected_capacity = expected_capacity * 2;
            }
        }

        const Message* before = conv.begin();
        conv.append(Message(Role::User, std::to_string(i)));
        const Message* after = conv.begin();

        bool grew = false;
        if (before != after) {
            grew = true;
        }

        assert(grew == should_grow);

        assert(conv.size() == i + 1);
        for (std::size_t j = 0; j <= i; j++) {
            assert(conv.at(j).content() == std::to_string(j));
        }
    }
}

void test6_clean_text() {
    const std::string text = "Hello! No sentinel here.";
    bool found = false;
    std::string result = "";

    // one chunk
    result = scan(text, text.size(), found);
    assert(result == text);
    assert(found == false);

    // byte by byte
    result = scan(text, 1, found);
    assert(result == text);
    assert(found == false);
}

void test7_split_everywhere() {
    const std::string text = "Goodbye." + SENTINEL;

    for (std::size_t split = 0; split <= text.size(); split++) {
        std::string first_part = text.substr(0, split);
        std::string second_part = text.substr(split);

        SentinelScanner scanner(SENTINEL);
        SentinelScanner::Out out1 = scanner.feed(first_part);
        SentinelScanner::Out out2 = scanner.feed(second_part);

        if (split < text.size()) {
            assert(out1.sentinel_found == false);  // no early match
        }

        bool found = false;
        if (out1.sentinel_found || out2.sentinel_found) {
            found = true;
        }
        assert(found == true);

        std::string all_output = out1.safe_text + out2.safe_text;
        assert(all_output == "Goodbye.");
    }
}

void test8_false_alarms() {
    const std::string fakes[] = {"<|end_world|>", "<|end_conversation", "<|end_conversation|"};
    bool found = false;
    std::string result = "";

    for (const std::string& text : fakes) {
        result = scan(text, 1, found);
        assert(result == text);
        assert(found == false);
    }

    // false start, then the real sentinel
    result = scan("<|end_" + SENTINEL, 1, found);
    assert(result == "<|end_");
    assert(found == true);
}

// pending_ is private, so we measure it from outside: characters fed minus characters returned = characters still held.
void test9_bounded_memory() {
    std::string stream = "";
    while (stream.size() < 4 * 1024 * 1024) {
        stream += "<|end_";  // almost matches, forever
    }

    SentinelScanner scanner(SENTINEL);
    std::size_t fed = 0;
    std::size_t returned = 0;
    std::size_t max_held = SENTINEL.size() - 1;

    for (char c : stream) {
        std::string one_char(1, c);

        SentinelScanner::Out out = scanner.feed(one_char);
        fed = fed + 1;
        returned = returned + out.safe_text.size();

        assert(out.sentinel_found == false);

        std::size_t held = fed - returned;
        assert(held <= max_held);
    }

    // nothing lost: flush gives back the rest
    SentinelScanner::Out leftover = scanner.flush();
    returned = returned + leftover.safe_text.size();
    assert(returned == fed);
}

void test10_turn_limit() {
    std::string script = "";
    script += "role: assistant\nreply one\n---\n";
    script += "role: assistant\nreply two\n---\n";
    script += "role: assistant\nreply three\n";

    std::string printed = "";
    StopReason reason = run_script(script, 2, printed);

    assert(reason.kind == StopReason::Kind::TurnLimit);
    assert(contains(printed, "reply one") == true);
    assert(contains(printed, "reply three") == false);
}

void test11_sentinel_halt() {
    std::string script = "";
    script += "role: assistant\nHi there.\n---\n";
    script += "chunk: 3\n";  // stream this reply 3 characters at a time
    script += "role: assistant\nGoodbye.<|end_conversation|>\n---\n";
    script += "role: assistant\nnever printed\n";

    std::string printed = "";
    StopReason reason = run_script(script, 20, printed);

    assert(reason.kind == StopReason::Kind::Sentinel);
    assert(contains(printed, "Goodbye.") == true);
    assert(contains(printed, "<|end") == false);          // sentinel hidden
    assert(contains(printed, "never printed") == false);  // stopped right away
}

// Save a transcript, replay it, and check every reply matches.
void test12_round_trip() {
    std::string reply1 = "Hi! How can I help?";
    std::string reply2 = "4.";

    std::string transcript = "";
    transcript += "role: user\nhello\n---\n";
    transcript += "role: assistant\n" + reply1 + "\n---\n";
    transcript += "role: user\nwhat is 2+2\n---\n";
    transcript += "role: assistant\n" + reply2 + "\n";

    std::string path = "test_transcript.txt";
    write_file(path, transcript);

    ReplayModelClient replay(path);
    ModelClient& model = replay;  // gives access to the simple generat

    Conversation conv;

    conv.append(Message(Role::User, "hello"));
    Message answer1 = model.generate(conv);
    assert(answer1.content() == reply1);

    conv.append(answer1);
    conv.append(Message(Role::User, "what is 2+2"));
    Message answer2 = model.generate(conv);
    assert(answer2.content() == reply2);

    std::remove(path.c_str());
}

int main() {
    test1_empty();
    std::cout << "PASS test 1\n";

    test2_system_first();
    std::cout << "PASS test 2\n";

    test3_copy();
    std::cout << "PASS test 3\n";

    test4_move();
    std::cout << "PASS test 4\n";

    test5_growth();
    std::cout << "PASS test 5\n";

    test6_clean_text();
    std::cout << "PASS test 6\n";

    test7_split_everywhere();
    std::cout << "PASS test 7\n";

    test8_false_alarms();
    std::cout << "PASS test 8\n";

    test9_bounded_memory();
    std::cout << "PASS test 9\n";

    test10_turn_limit();
    std::cout << "PASS test 10\n";

    test11_sentinel_halt();
    std::cout << "PASS test 11\n";

    test12_round_trip();
    std::cout << "PASS test 12\n";

    std::cout << "All tests passed.\n";
    return 0;
}