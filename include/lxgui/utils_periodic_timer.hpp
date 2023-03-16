#ifndef LXGUI_UTILS_PERIODIC_TIMER_HPP
#define LXGUI_UTILS_PERIODIC_TIMER_HPP

#include "lxgui/lxgui.hpp"

namespace lxgui::utils {

/**
 * \brief A repeating timer
 * \details This timer is meant to tick periodicaly,
 * so you can use it for any periodic event
 * such as key repetition or a count down.
 */
class periodic_timer {
public:
    enum class start_type {
        /// The timer will start if you call start()
        paused,
        /// The timer starts immediatly after it is created
        now,
        /// The timer will start when you first call ticks()
        first_tick
    };

    /**
     * \brief Default constructor
     * \param duration The time interval between each tick
     * \param type See TimerType
     * \param ticks_now The timer ticks immediately
     */
    periodic_timer(double duration, start_type type, bool ticks_now);

    /**
     * \brief Returns the time elapsed since the last tick.
     * \return The time elapsed since last tick
     */
    double get_elapsed() const;

    /**
     * \brief Returns the period of the periodic_timer.
     * \return The period of the periodic_timer
     */
    double get_period() const;

    /**
     * \brief Cheks if this periodic_timer is paused.
     * \return 'true' if this periodic_timer is paused
     */
    bool is_paused() const;

    /**
     * \brief Checks if the timer's period has been reached.
     * \return 'true' if the period has been reached
     */
    bool ticks();

    /// Pauses the timer and resets it.
    void stop();

    /// Starts the timer but doesn't reset it.
    void start();

    /// Pauses the timer.
    void pause();

    /// Resets the timer but doesn't pause it.
    void zero();

    /**
     * \brief Updates this timer (adds time).
     * \param delta The time elapsed since last update
     */
    void update(double delta);

private:
    double elapsed_    = 0.0;
    double duration_   = 0.0;
    bool   paused_     = true;
    bool   first_tick_ = true;

    start_type type_ = start_type::paused;
};

} // namespace lxgui::utils

#endif
