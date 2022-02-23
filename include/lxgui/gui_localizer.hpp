#ifndef LXGUI_GUI_LOCALIZER_HPP
#define LXGUI_GUI_LOCALIZER_HPP

#include "lxgui/gui_code_point_range.hpp"
#include "lxgui/lxgui.hpp"

#include <fmt/format.h>
#include <locale>
#include <lxgui/extern_sol2_state.hpp>
#include <lxgui/extern_sol2_variadic_args.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace lxgui::gui {

/// Utility class to translate strings for display in GUI.
class localizer {
public:
    /// Default constructor.
    localizer();

    // Non-copiable, non-movable
    localizer(const localizer&) = delete;
    localizer(localizer&&)      = delete;
    localizer& operator=(const localizer&) = delete;
    localizer& operator=(localizer&&) = delete;

    /**
     * \brief Changes the current locale (used to format numbers).
     * \param locale The new locale
     * \note This function should only be called before the UI is loaded. If you need to change
     * the locale after the UI has been loaded: close the UI, set the locale, and load the
     * UI again.
     */
    void set_locale(const std::locale& locale);

    /**
     * \brief Changes the current language (used to translate messages and strings).
     * \param languages A list of languages
     * \details This function specifies which languages to use for translating messages. The
     * languages listed in the supplied array will be tried one after the other, until a
     * translation is found. Each language in the list must be formatted as
     * "{language}{REGION}" with "{language}" a two-letter lower case word, and
     * "{REGION}" a two-letter upper case word. For example: "enUS" for United State English, and
     * "enGB" for British English, "frFR" for mainland French, etc. The default value is
     * {"enUS"}.
     * \note This function should only be called before the UI is loaded. If you need to change
     * the language after the UI has been loaded: close the UI, set the language, and load
     * the UI again.
     */
    void set_preferred_languages(const std::vector<std::string>& languages);

    /// Attempts to automatically detect the current language (used to translate messages and
    /**
     * \brief strings).
     * \note This is called in the constructor, only use it if you need to reset the languages
     * to the default after calling set_preferred_languages().
     */
    void auto_detect_preferred_languages();

    /**
     * \brief Returns the current locale (used to format numbers).
     * \return The current locale.
     */
    const std::locale& get_locale() const;

    /// Returns the list of code names of the preferred languages (used to translate messages and
    /**
     * \brief strings).
     * \return The list of code name of the preferred languages.
     */
    const std::vector<std::string>& get_preferred_languages() const;

    /**
     * \brief Removes all allowed code points.
     * \see get_allowed_code_points()
     */
    void clear_allowed_code_points();

    /**
     * \brief Adds a new range to the set of allowed code points.
     * \param range The new range to allow
     * \see get_allowed_code_points()
     */
    void add_allowed_code_points(const code_point_range& range);

    /**
     * \brief Adds a new range to the set of allowed code points from a Unicode group.
     * \param unicode_group The name of the Unicode code group to allow
     * \note The Unicode standard defines a set of code groups, which are contiguous
     * ranges of Unicode code points that are typically associated to a language
     * or a group of languages. This function knows about such groups and the
     * ranges of code point they correspond to, and is therefore more user-friendly.
     * \see get_allowed_code_points()
     */
    void add_allowed_code_points_for_group(const std::string& unicode_group);

    /**
     * \brief Adds a new range to the set of allowed code points for a given language.
     * \param language_code The language code (e.g., "en", "ru", etc.)
     * \note Language codes are based on the ISO-639-1 standard, or later standards for those
     * languages which were not listed in ISO-639-1. They are always in lower case, and
     * typically composed of just two letters, but sometimes more.
     * \see get_allowed_code_points()
     */
    void add_allowed_code_points_for_language(const std::string& language_code);

    /// Attempts to automatically detect the set of allowed code points based on preferred
    /**
     * \brief languages.
     * \note This is called in the constructor, only use it if you need to reset the allowed
     * code points to the default after changing the preferred languages.
     */
    void auto_detect_allowed_code_points();

    /**
     * \brief Returns the list of allowed code points (Unicode characters), for text rendering.
     * \return The list of allowed code points.
     * \note The list contains no duplicate or overlapping ranges. The ranges are sorted
     * by increasing code point value. This list is used by the font system to
     * pre-render all the characters that are allowed on the GUI, to allow for
     * more efficient rendering and batching. Code points or characters not on this
     * list will not be rendered, or will be replaced by a placeholder character.
     */
    const std::vector<code_point_range>& get_allowed_code_points() const;

    /**
     * \brief Sets the default character to display if a character is missing from a font.
     * \param code_point The Unicode UTF-32 code point of the character to display
     */
    void set_fallback_code_point(char32_t code_point);

    /**
     * \brief Returns the default character to display if a character is missing from a font.
     * \return The default character to display if a character is missing from a font
     */
    char32_t get_fallback_code_point() const;

    /**
     * \brief Loads new translations from a folder, selecting the language automatically.
     * \param folder_path The path to the folder to load translations from
     * \note Based on the current language (see get_language()), this function will scan files in
     * the supplied folder with name "{language}{REGION}.lua", and pick the combination of
     * "language" and "REGION" closest to the currently configured language.
     * It then loads translations from this file using load_translation_file() (see the
     * documentation of this function for more information).
     */
    void load_translations(const std::string& folder_path);

    /**
     * \brief Loads new translations from a file.
     * \param file_name The path to the file to load translations from
     * \note The file must be a Lua script. It will be loaded in a sandboxed Lua state
     * (independent of the Lua state of the GUI). The script must define a table called
     * "localize", which will be scanned to add new translations for the current locale
     * (each file must only contain translations for one language; separate different
     * languages into different files). Each item of the table must have a string key identifying
     * the localized content, e.g., "player_health". This key must be the same for all languages,
     * and serves to uniquely identify the translatable sentence / text. The format of this string
     * is arbitrary. The value of each element must be either a string, of a function. If the value
     * is a string, it must be a fmtlib format string. For example: "{0:L} HP" expects one input
     * argument (here a number representing the player's health). If the value of this argument is
     * 5000 and the locale is enUS, this will result in "5,000 HP". If the value is a Lua function,
     * it must take the same number of input values in all languages, and must return a translated
     * string. See localize() for more information on translation arguments.
     */
    void load_translation_file(const std::string& file_name);

    /**
     * \brief Removes all previously loaded translations.
     * \note After calling this function, it is highly recommended to always include at least
     * the Unicode groups "basic latin" (to render basic ASCII characters) and
     * "geometric shapes" (to render the "missing character" glyph).
     */
    void clear_translations();

    /**
     * \brief Translates a string with a certain number of arguments from Lua (zero or many).
     * \param message The string to format (e.g., "Player {0} has {1} HP.").
     * \param args A variadic list of formatting input arguments from a Sol Lua state.
     * \return The formatted string.
     * \details The string to format must follow the rules of libfmt format strings.
     */
    std::string format_string(std::string_view message, sol::variadic_args args) const;

    /**
     * \brief Translates a string with a certain number of arguments from C++ (zero or many).
     * \param message The string to format (e.g., "Player {0} has {1} HP.").
     * \param args A variadic list of formatting input arguments.
     * \return The formatted string.
     * \details The string to format must follow the rules of libfmt format strings.
     */
    template<typename... Args>
    std::string format_string(std::string_view message, Args&&... args) const {
        return fmt::format(locale_, message, std::forward<Args>(args)...);
    }

    /**
     * \brief Translates a string with a certain number of arguments from Lua (zero or many).
     * \param key The key identifying the sentence / text to translate (e.g.,
     * "{player_health}"). Must start with '{' and end with '}'.
     * \param args A variadic list of translation input arguments from a Sol Lua state.
     * \return The translated string, or key if not found or an error occurred.
     * \note See the other overload for more information.
     */
    std::string localize(std::string_view key, sol::variadic_args args) const;

    /**
     * \brief Translates a string with a certain number of arguments from C++ (zero or many).
     * \param key The key identifying the sentence / text to translate (e.g., "{player_health}").
     * Must start with '{' and end with '}'.
     * \param args A variadic list of translation input arguments.
     * \return The translated string, or key if not found or an error occurred.
     * \details This function will search the translation database (created from loading
     * translations with load_translations() or load_translation_file()) for a
     * string matching key. If one is found, it will forward the supplied arguments
     * to the translated formatting function, which will insert the arguments at
     * the proper place for the selected language.
     */
    template<typename... Args>
    std::string localize(std::string_view key, Args&&... args) const {
        if (!is_key_valid_(key))
            return std::string{key};

        auto iter = find_key_(key);
        if (iter == map_.end())
            return std::string{key};

        return std::visit(
            [&](const auto& item) {
                constexpr bool is_string =
                    std::is_same_v<std::decay_t<decltype(item)>, std::string>;
                if constexpr (is_string) {
                    if constexpr (sizeof...(Args) == 0)
                        return item;
                    else
                        return fmt::format(locale_, item, std::forward<Args>(args)...);
                } else {
                    auto result = item(std::forward<Args>(args)...);
                    if (result.valid() && result.begin() != result.end()) {
                        auto&& first = *result.begin();
                        if (first.template is<std::string>())
                            return first.template as<std::string>();
                        else
                            return std::string{key};
                    } else
                        return std::string{key};
                }
            },
            iter->second);
    }

    /**
     * \brief Registers this localizer on a Lua state.
     * \param lua The Lua lua to register on
     * \note Only one localizer object can be registered on any Lua state.
     * Registering enables Lua functions such as "get_locale()".
     */
    void register_on_lua(sol::state& lua);

private:
    using hash_type   = std::size_t;
    using mapped_item = std::variant<std::string, sol::protected_function>;
    using map_type    = std::unordered_map<hash_type, mapped_item>;

    std::locale                   locale_;
    std::vector<std::string>      languages_;
    std::vector<code_point_range> code_points_;
    char32_t                      default_code_point_ = U'\u25a1'; // '□'
    sol::state                    lua_;
    map_type                      map_;

    bool                     is_key_valid_(std::string_view key) const;
    map_type::const_iterator find_key_(std::string_view key) const;
    void                     reset_language_fallback_();
};

} // namespace lxgui::gui

#endif
