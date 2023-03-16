-- This is where you would put your global translations for US English.
-- Below are a few examples.

localize = {
    -- Simple text substitution
    ["window"] = "Window",
    -- Formatted text, with localized number format "{:L}"
    ["fps"] = "FPS: {:L}",
    -- Complex function, can have any number of input arguments
    ["file_count"] = function (count)
        if count == 0 then
            return "no file"
        elseif count == 1 then
            return "1 file"
        else
            return "{:L} files"
        end
    end,
}
