-- This is where you would put your global translations for mainland French.
-- Below are a few examples.

localize = {
    -- Simple text substitution
    ["window"] = "Fenêtre",
    -- Formatted text, with localized number format "{:L}"
    ["fps"] = "IPS: {:L}",
    -- Complex function, can have any number of input arguments
    ["file_count"] = function (count)
        if count == 0 then
            return "aucun fichier"
        elseif count == 1 then
            return "1 fichier"
        else
            return "{:L} fichiers"
        end
    end,
}
