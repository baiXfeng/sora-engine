
local function test(self)
    print("user function.")
end

function init(self)
    var[self].test = test
end

function on_assign(self, name, object)
    print("test1.lua assign", name, object)
end

function on_layout(self, parent, name, value)
    print("test1.lua layout", self, parent, name, value)
end
