#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_localizer.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

/** Global functions for localization / translation of text.
*   The functions listed on this page are registered in the
*   Lua state as globals, and as such are accessible from
*   anywhere automatically. They provide all necessary functionalities
*   for selecting languages and translating text for display in the GUI.
*
*   **Translating text.** When creating a button in the GUI, it is tempting to set its
*   displayed text directly like `button:set_text("Cancel")`, hence hard-coding the text
*   in the code. This works totally fine, provided that English is the only language you
*   only ever want to display to your users. If you want your game to be accessible to
*   users from countries where English is not the native language, you will want to
*   make that text translatable. This is what this module is for.
*
*   **Translatable string codes.** The first change needed is to stop hard-coding *any*
*   form of text in your GUI code (be it XML or Lua). All text should be replaced by a
*   "translatable string code", such as `"{button_cancel}"`. The purpose of this code is
*   to uniquely represent this string you need to display, without giving it an actual
*   content. The string `"{button_cancel}"` will not itself be displayed on the screen;
*   instead, the localization system will search for a translation of that string and
*   return the appropriate text (`"Cancel"` for English, `"Annuler"` for French, etc.).
*
*   The translation is automatic when the string code is set in the XML attributes,
*   like the `text` attribute of the `FontString` or `Button` objects. This is only
*   appropriate for translations that do not require additional inputs (see below).
*
*       <Button name="$parentButton" text="{button_cancel}">
*          ...
*       </Button>
*
*   When setting the string code from Lua, e.g. by calling `set_text("...")`, the
*   translation needs to be explicit. This is done by calling the @{localize_string}
*   function, for example:
*
*      button:set_text(localize_string("{button_cancel}"))
*
*   **Creating translations.** Once all the GUI code uses translatable string codes, all
*   that remains is to write the translations for each string, in all the languages you
*   want to support. The translation system is easily extensible: it is possible to add
*   more languages or more translatable text at any point. Therefore, even if you are
*   lacking translators for all the languages you need, it is a good practice to start
*   writing at least the English translations (or whatever is your native language),
*   and add the rest later.
*
*   Each addon folder is automatically scanned for translations when the GUI is loaded.
*   Translation files are Lua scripts, with name `{lang}{REGION}.lua`, where `{lang}`
*   is a two-letter lowercase string identifying the language (e.g., `en` for English)
*   and `{REGION}` is a two-letter uppercase string identifying the regional dialect
*   of this language (e.g., `US` for United States, `GB` for Great Britain, etc.).
*   The currently configured languages (see the @{get_preferred_languages} function)
*   determine which translation file gets loaded. If no translation file exists for the
*   preferred languages, the next best translation will be chosen by ignoring the
*   `{REGION}` code (i.e., if the preferred language is `enGB`, this enables automatic
*   fallback to `enUS` if `enGB` is not found). You can also add additional folders
*   to scan using the @{load_translations} function, and load a specific translation file
*   using the @{load_translation_file} function.
*
*   The format of a translation file is simple. It must be a Lua script defining a
*   table named `localize`. Each entry in this table will add a new translation to
*   the translation database, which is global to the entire GUI (hence, a translation
*   defined in one addon can be reused in another addon). An entry in the table must be
*   a key + value pair, where the key is the translatable string code (stripped of its
*   surrounding braces `{}`) and the value is the translation. The translation can be
*   specified as a simple formatted string (see below), or as a full blown Lua function.
*
*   If a translatable string code is duplicated in two addons, it will get over-written
*   by whichever addon is loaded last.
*
*   **Formatted translation strings.** While most translation strings will be simple
*   static strings to be displayed as-is, such as `"Cancel"`, it is also possible to
*   display dynamic data inside the translated text. One example would be displaying
*   a message like "Player1 has lost 10 HP.". There are two dynamic elements in this
*   sentence, "Player1" and "10", which cannot be hard-coded into the translation.
*   To support this, the translation must be done explicitly in Lua, by supplying
*   these two dynamic values to the translation function. For example:
*
*       local player = "Player1"
*       local hp_lost = 10
*       message:set_text(localize_string("{player_lost_hp}", player, hp_lost))
*
*   The translation must then refer to these dynamic elements to include them in the
*   displayed test. Dynamic elements are always indicated with pairs of braces `{}`.
*   Inside the braces, you can supply additional modifiers which will affect how the
*   data is being displayed. This follows the [fmtlib](https://fmt.dev/latest/syntax.html)
*   syntax. For example, the message above can be represented with the string
*
*       ["player_lost_hp"] = "{} has lost {} HP.",
*
*   By default, dynamic elements will be displayed in the same order as they are
*   supplied to the @{localize_string} function. You can override this by explicitly
*   specifying the position of the argument (recommended, even if the order
*   is the same):
*
*       ["player_lost_hp"] = "{0} has lost {1} HP.",
*
*   This allows re-using a single element multiple times, e.g.:
*
*       ["player_lost_hp"] = "{0} has lost {1} HP ({0} is hurt!).",
*
*   Numbers will be displayed with a very simple, locale-independent formatting
*   by default. This is fine for most uses (small integers), but might be confusing
*   when displaying fractional values, or very large numbers. In general, it is
*   preferable to rely on the user's locale for this, and this can be enabled
*   by the `L` modifier (recommended for displaying all numbers):
*
*       ["player_lost_hp"] = "{0} has lost {1:L} HP.",
*
*   **Translation functions.** In some cases, the translation is too complex to
*   be supported with the formatting syntax described above. This includes cases
*   where a word needs to be modified based on gender, quantity, or other properties,
*   in ways very specific to the user's language.
*
*   This is where translation functions can be used instead of strings. Reusing the
*   example from the previous section, an equivalent as a translation function could
*   be:
*
*       ["player_lost_hp"] = function (player, hp_lost)
*           return player.." has lost "..hp_lost.." HP."
*       end,
*
*   Now, we can actually change the displayed text based on how much HP have been lost:
*
*       ["player_lost_hp"] = function (player, hp_lost)
*           if hp_lost > 10 then
*               return player.." has lost "..hp_lost.." HP!!! Ouch."
*           elseif hp_lost == 0 then
*               return player.." has lost no HP. This had no effect."
*           else
*               return player.." has lost "..hp_lost.." HP."
*           end
*       end,
*
*   By default, the numbers printed this way may not be identical to the ones obtained
*   from translation strings such as `"{} has lost {} HP."`. This is because Lua
*   has its own number formatting rules. To ensure consistency with the rest of the
*   translations, you should use the @{format_string} function:
*
*       ["player_lost_hp"] = function (player, hp_lost)
*           if hp_lost > 10 then
*               return format_string("{0} has lost {1:L} HP!!! Ouch.",
                    player, hp_lost)
*           elseif hp_lost == 0 then
*               return format_string("{0} has lost no HP. This had no effect.",
                    player)
*           else
*               return format_string("{0} has lost {1:L} HP.", player, hp_lost)
*           end
*       end,
*
*   This can be used to implement complex support language rules like adding an "s" to
*   plurals of nouns in French:
*
*       -- Table for plurals that are not obtained by just adding "s"
*       special_plurals = {
*           ["cheval"] = "chevaux",
*           ["choux"] = "choux",
*           -- etc...
*       }
*
*       function plural_modifier(word, quantity)
*           if quantity > 1 then
*               local special = special_plurals[word]
*               if special ~= nil
*                   return special
*               else
*                   return word.."s"
*               end
*           else
*               return word
*           end
*       end
*
*       -- English translation would be: "Player1 ate 3 apples."
*       localize = {
*           ["object_apple"] = "pomme",
*           ["object_horse"] = "cheval",
*           ["object_cabbage"] = "choux",
*           ["player_ate_objects"] = function (player, quantity, object)
*               local object_name = localize_string(object)
*               if quantity == 0 then
*                   return format_string("{0} n'a pas mangé de {1}.",
*                       player, object_name)
*               else
*                   object_name = plural_modifier(object_name, quantity)
*                   return format_string("{0} a mangé {1} {2}.",
*                       player, quantity, object_name)
*               end
*           end,
*       }
*
*   **Language selection.** By default, the preferred languages are automatically detected
*   based on the environment and configuration of the operating system. The `LANGUAGE`
*   environment variable is scanned first, and can be used to set an ordered list of
*   preferred languages. This is commonly set on Linux and OSX systems, but rarely on
*   Windows. For Windows, if `LANGUAGE` is not set, the language configured in the operating
*   system is used. For other systems, the `LANG` environment variable is tried next. If
*   no language could be detected, the library falls back to US English. This automatic
*   detection can be overridden by calling @{set_preferred_languages}.
*
*   Once a language (or list of languages) is set, the GUI must be told which characters,
*   represented here as Unicode "code points", can be displayed on screen. This can be detected
*   automatically from the preferred languages by calling @{auto_detect_allowed_code_points}. This
*   function only needs to be called if the preferred languages were overridden with
*   @{set_preferred_languages}. If using the automatically-determined language from the environment
*   and operating system (see above), this is already done internally.
*
*   If the automatic detection of the allowed code points is incomplete, or if additional code
*   points need to be displayed for any reason (game-specific lore, etc.), it is possible to add
*   more code points to the allowed list with @{add_allowed_code_points_for_language},
*   @{add_allowed_code_points_for_group}, and @{add_allowed_code_points}. It is also possible to
*   reset the list of allowed code points using @{clear_allowed_code_points}, in the event that
*   the automatic detection was completely wrong.
*
*   **Encoding.** The Lua API only supports UTF-8 encoding for strings and files. Please make
*   sure to only use this encoding when writing scripts and translation files.
*
*   @module Localization
*/

namespace lxgui {
namespace gui
{
/** \cond NOT_REMOVE_FROM_DOC
*/

void localizer::register_on_lua(sol::state& mSol)
{
    /** Returns the preferred languages to display the GUI.
    *   @see set_preferred_languages
    *   @function get_preferred_languages
    *   @treturn table A table listing the languages selected to display the UI
    *
    */
    mSol.set_function("get_preferred_languages", [&]()
    {
        return sol::as_table(get_preferred_languages());
    });

    /** Sets the preferred languages to display the GUI.
    *   The languages must be listed in order of decreasing preference. They must be of the form
    *   `{language}{REGION}` where `{language}` is a two-letter lower case word identifying the
    *   main language, and `{REGION}` is a two-letter upper case word identifying the dialect or
    *   regional variant of the language (e.g., "enUS" for United States of America English, and
    *   "enGB" for Great Britain English). This change will not take effect until the GUI is
    *   re-loaded, see @{Manager.reload_ui}.
    *   @function set_preferred_languages
    *   @tparam table languages A table listing the languages to use to display the GUI
    */
    mSol.set_function("set_preferred_languages", [&]()
    {
        return sol::as_table(get_preferred_languages());
    });

    /** Removes all allowed code points.
    *   After calling this function, it is highly recommended to always include at least
    *   the Unicode groups "basic latin" (to render basic ASCII characters) and
    *   "geometric shapes" (to render the "missing character" glyph).
    *   This change will not take effect until the GUI is re-loaded, see @{Manager.reload_ui}.
    *   @function clear_allowed_code_points
    */
    mSol.set_function("clear_allowed_code_points", [&]()
    {
        clear_allowed_code_points();
    });

    /** Adds a new range to the set of allowed code points.
    *   This change will not take effect until the GUI is re-loaded, see @{Manager.reload_ui}.
    *   @tparam integer first The first code point in the range
    *   @tparam integer last The last code point in the range
    *   @function add_allowed_code_points
    */
    mSol.set_function("add_allowed_code_points", [&](char32_t uiFirst, char32_t uiLast)
    {
        add_allowed_code_points(code_point_range{uiFirst, uiLast});
    });

    /** Adds a new range to the set of allowed code points from a Unicode group.
    *   The Unicode standard defines a set of code groups, which are contiguous
    *   ranges of Unicode code points that are typically associated to a language
    *   or a group of languages. This function knows about such groups and the
    *   ranges of code point they correspond to, and is therefore more user-friendly.
    *   This change will not take effect until the GUI is re-loaded, see @{Manager.reload_ui}.
    *   @tparam string group The name of the Unicode code group to allow
    *   @function add_allowed_code_points_for_group
    */
    mSol.set_function("add_allowed_code_points_for_group", [&](const std::string& sGroupName)
    {
        add_allowed_code_points_for_group(sGroupName);
    });

    /** Adds a new range to the set of allowed code points for a given language.
    *   Language codes are based on the ISO-639-1 standard, or later standards for those
    *   languages which were not listed in ISO-639-1. They are always in lower case, and
    *   typically composed of just two letters, but sometimes more.
    *   This change will not take effect until the GUI is re-loaded, see @{Manager.reload_ui}.
    *   @tparam string language The language code (e.g., "en", "ru", etc.)
    *   @function add_allowed_code_points_for_language
    */
    mSol.set_function("add_allowed_code_points_for_language", [&](const std::string& sLanguage)
    {
        add_allowed_code_points_for_language(sLanguage);
    });

    /** Attempts to automatically detect the set of allowed code points based on preferred languages.
    *   Only use it if you need to reset the allowed code points to the default after changing
    *   the preferred languages with @{set_preferred_languages}.
    *   This change will not take effect until the GUI is re-loaded, see @{Manager.reload_ui}.
    *   @function auto_detect_allowed_code_points
    */
    mSol.set_function("auto_detect_allowed_code_points", [&]()
    {
        auto_detect_allowed_code_points();
    });

    /** Sets the default character to display if a character is missing from a font.
    *   @tparam integer character The Unicode UTF-32 code point of the character to display
    *   @function set_fallback_code_point
    */
    mSol.set_function("set_fallback_code_point", [&](char32_t uiCodePoint)
    {
        set_fallback_code_point(uiCodePoint);
    });

    /** Loads translations from a folder.
    *   @function load_translations
    *   @tparam string folder The folder to search for translations
    */
    mSol.set_function("load_translations", [&](const std::string& sFolderPath)
    {
        load_translations(sFolderPath);
    });

    /** Loads translations form a file.
    *   @function load_translation_file
    *   @tparam string filename The file from which to read new translations
    */
    mSol.set_function("load_translation_file", [&](const std::string& sFilename)
    {
        load_translation_file(sFilename);
    });

    /** Translate a string or message, with arguments.
    *   The arguments are passed as individual parameters after the string to translate.
    *   There can be as many arguments are needed (including zero).
    *   @function localize_string
    *   @tparam string message The translatable string code (e.g., "{player_health}")
    *   @param ... Data to display in the translatable string (e.g., the player's health value).
    *   @treturn string The translated message encoded as UTF-8.
    */
    mSol.set_function("localize_string", [&](const std::string& sKey, sol::variadic_args mVArgs)
    {
        return localize(sKey, mVArgs);
    });

    /** Format a string with arguments.
    *   The arguments are passed as individual parameters after the string to translate.
    *   There can be as many arguments are needed (including zero).
    *   @function format_string
    *   @tparam string message The string with formatting specifiers (e.g., "Player {0} has {1} HP.")
    *   @param ... Data to display in the formatted string (e.g., the player's health value).
    *   @treturn string The formatted string encoded as UTF-8.
    */
    mSol.set_function("format_string", [&](const std::string& sKey, sol::variadic_args mVArgs)
    {
        return format_string(sKey, mVArgs);
    });
}

}
}
