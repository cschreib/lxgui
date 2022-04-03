function ScrollTest:get_selected_file()
    return self.File:get_text()
end

function ScrollTest:init_root_folder()
    if self.initialized then
        return
    end

    self.folders = {}
    self.files = {}

    local rootFolder = create_frame("Button", "$parentRoot",
        self.Splitter.FolderFrame.Scroll, "ButtonTemplate_FolderButton")

    if not rootFolder then
        log("# Warning # : couldn't create root folder ?!")
        return
    end

    self.lastID = 1
    rootFolder.id = self.lastID
    rootFolder.developed = true
    rootFolder.parent = nil
    rootFolder.name = ""
    rootFolder.folder = "."
    rootFolder.level = 0
    rootFolder.folders = {}
    rootFolder.folderNum = 0

    rootFolder:set_text("/")
    rootFolder:set_anchor("RIGHT", self.Splitter.FolderFrame.Scroll, "RIGHT", -5, 0)
    rootFolder:set_anchor("TOP_LEFT", self.Splitter.FolderFrame.Scroll, "TOP_LEFT", 5, 5)

    rootFolder.Develop.Plus:hide()

    self.folders[self.lastID] = rootFolder

    local folderList = get_folder_list(".")

    if #folderList ~= 0 then
        rootFolder.Develop.Minus:show()
    end

    self.folderScrollChildHeight = 5 + 18 + 5

    for i, folder in pairs(folderList) do
        local folderButton = create_frame("Button", "$parentFolder"..(self.lastID + 1),
            rootFolder, "ButtonTemplate_FolderButton")

        if not folderButton then
            break
        end

        self.lastID = self.lastID + 1
        folderButton.id = self.lastID
        folderButton.developed = false
        folderButton.parent = rootFolder
        folderButton.name = folder
        folderButton.folder = folder
        folderButton.level = 1
        folderButton.folders = {}
        folderButton.folderNum = 0

        folderButton:set_text(folder)
        local font = folderButton:get_normal_font_object()
        font:set_word_wrap(false)

        if rootFolder.lastFolder then
            rootFolder.lastFolder.nextFolder = folderButton
            folderButton:set_anchor("TOP_LEFT", rootFolder.lastFolder, "BOTTOM_LEFT")
        else
            folderButton:set_anchor("TOP_LEFT", rootFolder, "BOTTOM_LEFT", 16, 0)
        end

        self.folders[self.lastID] = folderButton
        rootFolder.folders[self.lastID] = folderButton
        rootFolder.folderNum = rootFolder.folderNum + 1
        rootFolder.lastFolder = folderButton
        self.folderScrollChildHeight = self.folderScrollChildHeight + 18
    end

    rootFolder.folderNum = #rootFolder.folders

    self.Splitter.FolderFrame.Scroll:set_height(self.folderScrollChildHeight)

    self.initialized = true
end

function ScrollTest:develop_folder(id, toggle)
    if not self.folders[id] then
        log("# Warning # : ScrollTest : No folder with ID : "..id)
        return
    end

    local parentFolder = self.folders[id]
    if not parentFolder.developed then
        parentFolder.Develop.Plus:hide()
        parentFolder.developed = true

        local folderList = get_folder_list(parentFolder.folder)

        if #folderList == 0 then
            parentFolder.isEmpty = true
            parentFolder.Develop:hide()
            return
        end

        parentFolder.isEmpty = false
        parentFolder.Develop.Minus:show()

        for i, folder in pairs(folderList) do
            local folderButton = create_frame("Button", "$parentFolder"..(self.lastID + 1),
                parentFolder, "ButtonTemplate_FolderButton")

            if not folderButton then
                break
            end

            self.lastID = self.lastID + 1
            folderButton.id = self.lastID
            folderButton.developed = false
            folderButton.parent = parentFolder
            folderButton.name = folder
            if #parentFolder.folder ~= 0 then
                folderButton.folder = parentFolder.folder.."/"..folder
            else
                folderButton.folder = folder
            end

            folderButton.level = parentFolder.level + 1
            folderButton.folders = {}
            folderButton.folderNum = 0

            folderButton:set_text(folder)
            folderButton:get_normal_font_object():set_word_wrap(false)

            if parentFolder.lastFolder then
                parentFolder.lastFolder.nextFolder = folderButton
                folderButton:set_anchor("TOP_LEFT", parentFolder.lastFolder, "BOTTOM_LEFT")
            else
                folderButton:set_anchor("TOP_LEFT", parentFolder, "BOTTOM_LEFT", 16, 0)
            end

            self.folders[self.lastID] = folderButton
            parentFolder.folders[self.lastID] = folderButton
            parentFolder.folderNum = parentFolder.folderNum + 1
            parentFolder.lastFolder = folderButton
        end

        local parent = parentFolder.parent
        while parent do
            parent.folderNum = parent.folderNum + parentFolder.folderNum
            parent = parent.parent
        end

        self.folderScrollChildHeight = self.folderScrollChildHeight + 18*parentFolder.folderNum
        self.Splitter.FolderFrame.Scroll:set_height(self.folderScrollChildHeight)
        if self.Splitter.FolderFrame:get_height() then
            self.Splitter.FolderFrame.Slider:set_min_max_values(
                0, math.max(0, self.folderScrollChildHeight - self.Splitter.FolderFrame:get_height())
            )
        end

        if parentFolder.nextFolder then
            parentFolder.nextFolder:set_anchor(
                "TOP_LEFT", parentFolder.lastFolder, "BOTTOM_LEFT",
                -16*(parentFolder.lastFolder.level - parentFolder.nextFolder.level), 0
            )

            parentFolder.lastFolder.nextFolder = parentFolder.nextFolder
        end
    elseif not parentFolder.isEmpty and toggle then
        parentFolder.Develop.Minus:hide()
        parentFolder.Develop.Plus:show()
        parentFolder.developed = false

        if (parentFolder.lastFolder.nextFolder) then
            parentFolder.lastFolder.nextFolder:set_anchor("TOP_LEFT", parentFolder, "BOTTOM_LEFT")
            parentFolder.nextFolder = parentFolder.lastFolder.nextFolder
        end

        parentFolder.lastFolder = nil

        local parent = parentFolder.parent
        while parent do
            parent.folderNum = parent.folderNum - parentFolder.folderNum
            parent = parent.parent
        end

        self.folderScrollChildHeight = self.folderScrollChildHeight - 18*parentFolder.folderNum
        self.Splitter.FolderFrame.Scroll:set_height(self.folderScrollChildHeight)
        self.Splitter.FolderFrame.Slider:set_min_max_values(
            0, math.max(0, self.folderScrollChildHeight - self.Splitter.FolderFrame:get_height())
        )

        if self.currentFolder then
            local parent = self.currentFolder.parent
            while parent do
                if parent == parentFolder then
                    self:set_folder(parentFolder.id)
                    self:set_file(nil)
                    break
                end
                parent = parent.parent
            end
        end

        for i, folder in pairs(parentFolder.folders) do
            delete_frame(folder)
        end

        parentFolder.folders = {}
        parentFolder.folderNum = 0
    end
end

function ScrollTest:set_folder(id)
    if not id and self.currentFolder then
        self:set_file(nil)

        for i, file in pairs(self.files) do
            delete_frame(file)
        end

        self.files = {}

        self.currentFolder:set_backdrop(nil)

        self.currentFolder = nil
        self.lastFile = nil

        return
    end

    if not self.folders[id] then
        log("# Warning # : ScrollTest : No folder with ID : "..id)
        return
    end

    local parentFolder = self.folders[id]

    if parentFolder == self.currentFolder then
        return
    end

    self:set_file(nil)

    for i, file in pairs(self.files) do
        delete_frame(file)
    end

    self.lastFile = nil
    self.files = {}

    self.File:set_text(parentFolder.folder)

    local fileList = get_file_list(parentFolder.folder)

    if self.currentFolder then
        self.currentFolder:set_backdrop(nil)
    end

    self.currentFolder = parentFolder
    self.fileScrollChildHeight = 10

    parentFolder:set_backdrop({edgeFile = "|border_1px.png"})
    parentFolder:set_backdrop_color(1, 1, 1, 0.35)

    if #fileList == 0 then
        parentFolder.isFileEmpty = true
        return
    end

    local fileID = 0

    for i, file in pairs(fileList) do
        local fileButton = create_frame("Button", "$parentFile"..(fileID + 1), self.Splitter.FileFrame.Scroll, "ButtonTemplate_FileButton")
        if not fileButton then
            break
        end

        fileID = fileID + 1
        fileButton.id = fileID
        fileButton.name = file
        if #parentFolder.folder ~= 0 then
            fileButton.file = parentFolder.folder.."/"..file
        else
            fileButton.file = file
        end
        fileButton:set_text(file)
        fileButton:get_normal_font_object():set_word_wrap(false)

        if self.lastFile then
            fileButton:set_anchor("TOP_LEFT", self.lastFile, "BOTTOM_LEFT")
        else
            fileButton:set_anchor("TOP_LEFT", self.Splitter.FileFrame.Scroll, "TOP_LEFT", 5, 5)
        end

        fileButton.IconFrame.Icon:set_texture("|icons.png")

        local dotPos = string.find(file, ".", 0, true)
        local extension
        if dotPos then
            extension = string.sub(file, dotPos+1)
        end

        if extension then
            if extension == "lua" or extension == "xml" or extension == "toc" or extension == "def"
                or extension == "txt" or extension == "glsl" or extension == "hlsl" then
                fileButton.IconFrame.Icon:set_tex_coord(0.625, 0, 0.75, 1)
            elseif extension == "png" or extension == "jpg" or extension == "tga" or extension == "bmp"
                    or extension == "gif" then
                fileButton.IconFrame.Icon:set_tex_coord(0.75, 0, 0.875, 1)
            elseif extension == "fm" or extension == "ft" or extension == "mesh" then
                fileButton.IconFrame.Icon:set_tex_coord(0.375, 0, 0.5, 1)
            else
                fileButton.IconFrame.Icon:set_tex_coord(0.5, 0, 0.625, 1)
            end
        else
            fileButton.IconFrame.Icon:set_tex_coord(0.5, 0, 0.625, 1)
        end

        self.files[fileID] = fileButton
        self.lastFile = fileButton
        self.fileScrollChildHeight = self.fileScrollChildHeight + 18
    end

    self.Splitter.FileFrame.Scroll:set_height(self.fileScrollChildHeight)
    self.Splitter.FileFrame.Slider:set_min_max_values(
        0, math.max(0, self.fileScrollChildHeight - self.Splitter.FileFrame:get_height())
    )
end

function ScrollTest:set_file(id)
    if not id and self.currentFile then
        self.currentFile:set_backdrop(nil)
        self.currentFile = nil
    else
        local file
        if id then
            file = self.files[id]
            if not file then
                log("# Warning # : ScrollTest : No file with ID : "..id)
            end
        else
            file = nil
        end

        if self.currentFile ~= file then
            if self.currentFile then
                self.currentFile:set_backdrop(nil)
            end

            self.currentFile = file

            if self.currentFile then
                self.currentFile:set_backdrop({edgeFile = "|border_1px.png"})
                self.currentFile:set_backdrop_color(1, 1, 1, 0.35)

                self.File:set_text(file.file)
            else
                self.File:set_text("")
            end
        end
    end
end

function ScrollTest:reset()
    for i, folder in pairs(self.folders[1].folders) do
        if folder.developed then
            if folder.isEmpty then
                folder.Develop:show()
                folder.Develop.Plus:show()
                folder.developed = false
            else
                self:develop_folder(folder.id)
            end
        end
    end
end
