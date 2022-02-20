#include "lxgui/utils_periodic_timer.hpp"

namespace lxgui::utils {

periodic_timer::periodic_timer(double duration, start_type type, bool ticks_now) :
    elapsed_(ticks_now ? duration : 0.0), duration_(duration), type_(type) {
    if (type == start_type::now)
        start();
}

double periodic_timer::get_elapsed() const {
    return elapsed_;
}

double periodic_timer::get_period() const {
    return duration_;
}

bool periodic_timer::is_paused() const {
    return paused_;
}

bool periodic_timer::ticks() {
    if (type_ == start_type::first_tick && first_tick_) {
        start();
        first_tick_ = false;
    }

    if (elapsed_ >= duration_) {
        if (!paused_)
            zero();

        return true;
    } else
        return false;
}

void periodic_timer::stop() {
    elapsed_ = 0.0;
    paused_  = true;
}

void periodic_timer::pause() {
    paused_ = true;
}

void periodic_timer::start() {
    paused_ = false;
}

void periodic_timer::zero() {
    elapsed_ = 0.0;
}

void periodic_timer::update(double delta) {
    elapsed_ += delta;
}

} // namespace lxgui::utils
