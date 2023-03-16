-- Simple class creation.
-- Link : http://lua-users.org/wiki/SimpleLuaClasses
-------
function CreateClass(base, ctor)
    local c = {};     -- A new class instance
    if (not(ctor) and (type(base) == 'function')) then
        ctor = base;
        base = nil;
    elseif (type(base) == 'table') then
        -- Our new class is a shallow copy of the base class!
        for i,v in pairs(base) do
            c[i] = v;
        end
        c._base = base;
    end
    -- The class will be the metatable for all its objects,
    -- And they will look up their methods in it.
    c.__index = c;

    -- Expose a ctor which can be called by <classname>(<args>)
    local mt = {};
    mt.__call = function (class_tbl, ...)
        local obj = {};
        setmetatable(obj, c);
        if (ctor) then
            ctor(obj, ...);
        else
            -- Make sure that any stuff from the base class is initialized!
            if (base and base.init) then
                base.init(obj, ...);
            end
        end
        return obj;
    end

    c.init = ctor;
    c.is_a = function(self, class)
        local m = getmetatable(self);
        while (m) do
            if (m == class) then
                return true;
            end
            m = m._base;
        end
        return false;
    end
    setmetatable(c, mt);
    return c;
end
