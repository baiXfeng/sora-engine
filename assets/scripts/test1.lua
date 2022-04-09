
local function test(self)
    print("user function.")
end

function init(self)
    self.test = test
end

function on_layout(self, parent, name, value)
    print("test1.lua", self, parent, name, value)
end
