
local c = {}

local function test(self)
    print("test function.")

    local steps = action.Steps(
            action.FadeBy(-255, 1.0),
            action.FadeBy(255, 1.0)
    )
    local act = action.Repeat(steps, 0)
    act.name = "fade"

    self:stopAction("fade")
    --self:runAction(act)
    self:setOpacity(255)

    local rotate = action.Repeat(action.RotationBy(360, 3.0))
    rotate.name = "rotate"

    self:stopAction("rotate")
    self:runAction(rotate)
    self:setRotation(0)

    self:setPosition({x=480, y=272})
    self:setAnchor({x=0.5, y=0.5})
    --self:setRenderTarget(true)

    return
    --[[
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

    c.bg:stopAction("rotate")
    c.bg:runAction(rotateBg)
    c.bg:setRotation(0)
    ]]--
end

function init(self)
    print("init")

    test(self)

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
    test(self)
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
    c[name] = object
    if name == "bg" then
        local size = self:size()
        object:setPosition({x=size.x*0.5, y=size.y*0.5})
        object:setAnchor({x=0.5, y=0.5})
    elseif name == "layout1" then
        var[object].test(object)
    end
end

function on_layout(self, parent, name, value)
    print(self, parent, name, value)
end

--collectgarbage("collect")
