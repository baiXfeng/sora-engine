
local function test(self)
    print("test function.")
    local act = action.Steps(
            action.Blink(10, 1.5),
            action.Callback(function()
                print("callback.")
            end),
            action.MoveBy({100, 100}, 1.0),
            action.MoveBy({-100, -100}, 1.0)
    )
    act.name = "test"
    self:stopAction("test")
    self:runAction(act)
    self:setPosition({x=0, y=0})
    self:setVisible(true)

    local rotateBg = action.Steps(
            action.Delay(3.0),
            action.RotationBy(90, 1),
            action.RotationBy(-90, 1)
    )
    rotateBg.name = "rotate"
    self.bg:stopAction("rotate")
    self.bg:runAction(rotateBg)
    self.bg:setRotation(0)
end

function init(self)
    print("init")
    self.act = test
    self:act()

    --[[self.node = self:add("assets/layouts/test.xml")
    if self.node ~= nil then
        local size = self:size()
        self.node:setPosition({x=size.x*0.5, y=size.y*0.5})
    end]]--
end

function release(self)

end

function update(self, dt)

end

function key_down(self, key)
    print("key down", key)
    print("parent", self.bg:parent(), self, self:parent())
    self.bg:parent():act()
end

function key_up(self, key)
    print("key up", key)
end

function joy_stick(self, joyid, point)
    print("joy stick", joyid, point.x, point.y)
end

function touch_began(self, point)
    --print("touch began", point.x, point.y)
    self:act()
    return true
end

function touch_moved(self, point)
    --print("touch moved", point.x, point.y)
end

function touch_ended(self, point)
    --print("touch ended", point.x, point.y)
end

function on_assign(self, name, object)
    print("obj.lua assign", name, object)
    self[name] = object
    if name == "bg" then
        local size = self:size()
        object:setPosition({x=size.x*0.5, y=size.y*0.5})
        object:setAnchor({x=0.5, y=0.5})
    elseif name == "layout1" then
        object:test()
    end
end

function on_layout(self, parent, name, value)
    print(self, parent, name, value)
end

--collectgarbage("collect")
