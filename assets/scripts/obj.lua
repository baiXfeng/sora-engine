
function init(self)
    print("init")
    self.a = "a"
    print("a = ", self.a)
    self.test1 = test1
    collectgarbage("collect")
    print("self nil")

    print("scene.push", scene.push)
    print("global push", push)
end

local bb = "bb."

function release(self)
    print("release start")
    print("a = ", self.a)
    --self.b()
    self.test1()
    print(bb)
    print("release end")
end

function update(self, dt)
    print("update start")
    print(self, dt)
    print("update end")
end

function test1()
    print("test1")
end

--collectgarbage("collect")
