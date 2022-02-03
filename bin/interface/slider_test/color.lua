-- Color class definition
Color = CreateClass(function (color, r, g, b, a)
    if (not g and not b and not a) then
        g = r.g; b = r.b; a = r.a; r = r.r;
    end
    color.r = math.min(math.max(r, 0), 1);
    color.g = math.min(math.max(g, 0), 1);
    color.b = math.min(math.max(b, 0), 1);
    color.h = 0.0; color.s = 0.0; color.l = 0.0;
    if (a) then color.a = math.min(math.max(a, 0), 1); else color.a = 1; end
end);

-- Operators overloadings
function Color.__add(c1, c2)
    if (type(c2) == "number") then
        return Color(c1.r+c2, c1.g+c2, c1.b+c2, c1.a);
    else
        return Color(c1.r+c2.r, c1.g+c2.g, c1.b+c2.b, c1.a);
    end
end

function Color.__sub(c1, c2)
    if (type(c2) == "number") then
        return Color(c1.r-c2, c1.g-c2, c1.b-c2, c1.a);
    else
        return Color(c1.r-c2.r, c1.g-c2.g, c1.b-c2.b, c1.a);
    end
end

function Color.__mul(c1, c2)
    if (type(c2) == "number") then
        return Color(c1.r*c2, c1.g*c2, c1.b*c2, c1.a);
    else
        return Color(c1.r*c2.r, c1.g*c2.g, c1.b*c2.b, c1.a*c2.a);
    end
end

function Color.__div(c1, c2)
    if (type(c2) == "number") then
        return Color(c1.r/c2, c1.g/c2, c1.b/c2, c1.a);
    else
        return Color(c1.r/c2.r, c1.g/c2.g, c1.b/c2.b, c1.a/c2.a);
    end
end

function Color.__concat(str, c)
    return str.."a:"..math.floor(c.a*255)
        ..", r:"..math.floor(c.r)
        ..", g:"..math.floor(c.g)
        ..", b:"..math.floor(c.b)
        .." (h:"..math.floor(c.h)
        ..", s:"..math.floor(c.s)
        ..", l:"..math.floor(c.l)..")";
end

-- Member functions
function Color:assign(r, g, b, a)
    if (not g and not b and not a) then
        g = r.g; b = r.b; a = r.a; r = r.r;
    end
    self.r = math.min(math.max(r, 0), 1);
    self.g = math.min(math.max(g, 0), 1);
    self.b = math.min(math.max(b, 0), 1);
    if (a) then self.a = math.min(math.max(a, 0), 1); else self.a = 1; end
end

function Color:unpack()
    return self.r, self.g, self.b, self.a;
end

function Color:serialize(tab)
    return "Color("..self.r..", "..self.g..", "..self.b..", "..self.a..")";
end

function Color:make_HSL_from_RGB()
    -- Algorithm taken from :
    -- http://en.wikipedia.org/wiki/HSV_color_space#Conversion_from_RGB_to_HSL_or_HSV
    -- http://130.113.54.154/~monger/hsl-rgb.html
    local ma = math.max(math.max(self.r, self.g), self.b);
    local mi = math.min(math.min(self.r, self.g), self.b);

    if (ma == mi) then
        self.l = ma;
        self.s = 0.0;
    else
        local delta = ma - mi;
        local sum   = ma + mi;
        
        self.l = 0.5*sum;
        if (self.l < 0.5) then self.s = delta/sum;
        else                   self.s = delta/(2.0 - sum);
        end
        
        if     (ma == self.r) then self.h = 60.0*(self.g - self.b)/delta +   0.0;
        elseif (ma == self.g) then self.h = 60.0*(self.b - self.r)/delta + 120.0;
        elseif (ma == self.b) then self.h = 60.0*(self.r - self.g)/delta + 240.0;
        end
        
        if     (self.h < 0.0)   then self.h = self.h + 360.0;
        elseif (self.h > 360.0) then self.h = self.h - 360.0;
        end
    end
end

local function H2RGB(v1, v2, h)
    if     (h < 0.0)   then h = h + 360.0;
    elseif (h > 360.0) then h = h - 360.0;
    end
    
    if     (h < 60)  then return v1 + (v2 - v1)*h/60.0;
    elseif (h < 180) then return v2;
    elseif (h < 240) then return v1 + (v2 - v1)*(4.0 - h/60.0);
    else                  return v1;
    end
end

function Color:make_RGB_from_HSL()
    -- Algorithm taken from :
    -- http://en.wikipedia.org/wiki/HSV_color_space#Conversion_from_HSL_to_RGB
    -- http://130.113.54.154/~monger/hsl-rgb.html
    if (self.s == 0.0) then
        self.r = self.l; self.g = self.l; self.b = self.l;
    else
        local v2;
        if (self.l < 0.5) then
            v2 = self.l*(1.0 + self.s);
        else
            v2 = self.l + self.s - self.l*self.s;
        end
        
        local v1 = 2.0*self.l - v2;
        
        self.r = H2RGB(v1, v2, self.h + 120.0);
        self.g = H2RGB(v1, v2, self.h);
        self.b = H2RGB(v1, v2, self.h - 120.0);
    end
end
