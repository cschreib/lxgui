FSTest = {
    ["updateTimer"] = 1, -- Time to wait between each update of the FPS
    ["frameNbr"]    = 0, -- Frame counter
    ["FPS"]         = 0, -- The FPS as it is displayed on the screen

    ["totalRenderTime"] = 0, -- Total render time between each update
    ["totalUpdateTime"] = 0, -- Total render time between each update
    ["renderTime"]      = 0, -- Render time displayed on screen
    ["updateTime"]      = 0, -- Render time displayed on screen
};

function FSTest.on_update(dt)
    FSTest.updateTimer = FSTest.updateTimer + dt;
    FSTest.frameNbr = FSTest.frameNbr + 1;

    if (FSTest.updateTimer >= 0.5) then
        FSTest.FPS = math.floor(FSTest.frameNbr / FSTest.updateTimer);

        FSTest.updateTimer = 0;
        FSTest.frameNbr = 0;
    end

    FontstringTestFrameText:set_text("FPS : "..FSTest.FPS.."\n(created in XML/Lua)");
end

FSTest.big_text = "Hello and welcome to the |cFF3FA7F3GUI test program|r !\n\n"
    .."Here is the list of all GUI elements that you should see on the screen :\n"
    .."  - a |cFFFFC109simple texture|r (like this |Tinterface/texture_test/texture.png|t) "
    .."and a |cFFFFC109small gradient|r above it. "
    .."They are located in the bottom left corner of the screen, right below this text.\n"
    .."  - an |cFFFFC109FPS meter|r, and an |cFFFFC109FPS gauge|r, at the bottom right "
    .."corner of the screen.\n"
    .."  - a |cFFFFC109big shadowed text|r at the top right corner of the screen.\n"
    .."  - a |cFFFFC109clickable status bar|r at the bottom of the screen.\n"
    .."  - a |cFFFFC109color selector|r at the top left corner of the screen.\n"
    .."  - a |cFFFFC109file selector|r in the middle.\n\n"
    .."If you see all these elements, then voilà : |cFF3FA7F3you have successfully "
    .."installed the GUI library !|r";

FSTest.controls = "|cFF3FA7F3Controls|r\n\n"
    .."Exit the test : |cFFFFC109[Escape]|r - \n"
    .."Toggle caching : |cFFFFC109[C]|r - \n"
    .."Reload the GUI : |cFFFFC109[R]|r - \n"
    .."Serialize the GUI in gui.log : |cFFFFC109[P]|r - \n\n"
    .."Also note that you can interact with the two frames (the color selector "
    .."and the file browser) : you can move them by dragging their title bar ("
    .."also called 'TitleRegion'), and you can resize them by dragging their edges."
