
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

    local m = var[self]
    m.bg:stopAction("rotate")
    m.bg:runAction(rotateBg)
    m.bg:setRotation(0)
end

function init(self)
    print("init")

    local m = var[self]
    --m.act = test
    --m.act(self)

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
end

function key_up(self, key)
    print("key up", key)
end

function joy_stick(self, joyid, point)
    print("joy stick", joyid, point.x, point.y)
end

function keyboard_down(self, key)
    print("keyboard down", key)
end

function keyboard_up(self, key)
    print("keyboard up", key)
end

function mouse_down(self, state)
    print("mouse_down", state.x, state.y)
    return true
end

function mouse_enter(self, state)
    print("mouse enter", state.x, state.y)
end

function mouse_exit(self, state)
    print("mouse exit", state.x, state.y)
end

function mouse_wheel(self, state)
    print("mouse wheel", state.x, state.y)
    return true
end

function on_assign(self, name, object)
    print("obj.lua assign", name, object)
    local m = var[self]
    m[name] = object
    if name == "bg" then
        local size = self:size()
        object:setPosition({x=size.x*0.5, y=size.y*0.5})
        object:setAnchor({x=0.5, y=0.5})
    elseif name == "layout1" then
        local o = var[object]
        o.test(object)
    end
end

function on_layout(self, parent, name, value)
    print(self, parent, name, value)
end

--collectgarbage("collect")
