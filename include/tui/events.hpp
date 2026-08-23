#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>

namespace tui {

enum class EventType {
    Keyboard,
    Mouse,
    Resize,
    Unknown,
};

struct Mouse {
    int  x    = 0;
    int  y    = 0;
    bool drag = false;
};

struct Resize {
    int width  = 0;
    int height = 0;
};

// Event holds everything about one user interaction.
// id is a human-readable string like "<Up>", "<C-c>", "a", "<Resize>".
struct Event {
    EventType   type = EventType::Unknown;
    std::string id;
    Mouse       mouse;   // only valid when type == Mouse
    Resize      resize;  // only valid when type == Resize
};

// EventChannel is a thread-safe queue that the event-polling thread
// writes to and the application reads from.
class EventChannel {
public:
    // Block until the next event is available.
    Event pop();

    // Non-blocking check — returns false if nothing is queued.
    bool tryPop(Event& out);

    void push(Event e);

    // Signal the channel to stop (unblocks any waiting pop()).
    void stop();
    bool stopped() const { return stopped_; }

private:
    std::queue<Event>           queue_;
    std::mutex                  mutex_;
    std::condition_variable     cv_;
    bool                        stopped_ = false;
};

// Start a background thread that reads ncurses input and pushes Events
// into the returned channel.  Call channel->stop() to shut down.
std::shared_ptr<EventChannel> pollEvents();

} // namespace tui
