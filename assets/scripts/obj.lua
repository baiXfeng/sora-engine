
local function test()
    print("test function.")
end

function init(self)
    print("init")
    self.bg.test()

    self.bg:runAction(action.Steps(
            action.Blink(10, 1.5),
            action.Callback(function()
                print("callback.")
            end),
            action.MoveBy({100, 100}, 1.0),
            action.MoveBy({-100, -100}, 1.0)
    ))
    defer(self, test, 5)
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
    print("joystick", joyid, point.x, point.y)
end

function touch_began(self, point)
    --print("touch began", point.x, point.y)
    return true
end

function touch_moved(self, point)
    --print("touch moved", point.x, point.y)
end

function touch_ended(self, point)
    --print("touch ended", point.x, point.y)
end

local function test(self)
    print("test.", self)
end

function on_assign(self, name, object)
    print(name, object)
    self[name] = object
    if name == "bg" then
        object.test = test
    end
end

--collectgarbage("collect")
