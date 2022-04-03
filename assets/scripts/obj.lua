
function init(self)

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

function touch_began(self, point)
    print("touch began", point.x, point.y)
    return true
end

function touch_moved(self, point)
    print("touch moved", point.x, point.y)
end

function touch_ended(self, point)
    print("touch ended", point.x, point.y)
end

--collectgarbage("collect")
