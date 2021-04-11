function ColorSelector:update_A_slider()
    self.SliderA.Slider:set_value(self.currentColor.a*255.0, true);
    self.SliderA.ValueString:set_text(self.currentColor.a*255.0);
end

function ColorSelector:update_RGB_sliders()
    self.SliderR.Slider:set_value(self.currentColor.r*255.0, true);
    self.SliderG.Slider:set_value(self.currentColor.g*255.0, true);
    self.SliderB.Slider:set_value(self.currentColor.b*255.0, true);

    self.SliderR.ValueString:set_text(math.floor(self.currentColor.r*255.0 + 0.5));
    self.SliderG.ValueString:set_text(math.floor(self.currentColor.g*255.0 + 0.5));
    self.SliderB.ValueString:set_text(math.floor(self.currentColor.b*255.0 + 0.5));
end

function ColorSelector:update_HSL_sliders()
    self.SliderH.Slider:set_value(self.currentColor.h,       true);
    self.SliderS.Slider:set_value(self.currentColor.s*100.0, true);
    self.SliderL.Slider:set_value(self.currentColor.l*100.0, true);

    self.SliderH.ValueString:set_text(math.floor(self.currentColor.h + 0.5));
    self.SliderS.ValueString:set_text(math.floor(self.currentColor.s*100.0 + 0.5));
    self.SliderL.ValueString:set_text(math.floor(self.currentColor.l*100.0 + 0.5));
end

function ColorSelector:set_color(color)
    self.currentColor = Color(color);

    -- Set the value of classic color sliders
    self:update_A_slider();
    self:update_RGB_sliders();

    self.Swatch.ColorZone:set_backdrop_color(self.currentColor:unpack());

    -- Calculate HSV values
    self.currentColor:make_HSL_from_RGB();
    self:update_HSL_sliders();

    -- Update the swatch color
    self.Swatch.ColorZone:set_backdrop_color(self.currentColor:unpack());
end

function ColorSelector:on_color_update(chanel, value)
    -- Update currentColor and make the conversion between RGB and HLS
    if (chanel == "r" or chanel == "g" or chanel == "b") then
        self.currentColor[chanel] = value/255.0;
        self.currentColor:make_HSL_from_RGB();
        self:update_HSL_sliders();
    elseif (chanel == "a") then
        self.currentColor[chanel] = value/255.0;
    elseif (chanel == "h") then
        self.currentColor[chanel] = value;
        self.currentColor:make_RGB_from_HSL();
        self:update_RGB_sliders();
    elseif (chanel == "s" or chanel == "l") then
        self.currentColor[chanel] = value/100.0;
        self.currentColor:make_RGB_from_HSL();
        self:update_RGB_sliders();
    end

    -- Update the swatch color
    self.Swatch.ColorZone:set_backdrop_color(self.currentColor:unpack());
end
