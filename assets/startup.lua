
import("assets/lualibs/sugar.lua")
import("assets/lualibs/action.lua")
import("assets/lualibs/variable.lua")

scene.push("assets/layouts/title.xml")

local _var = {}
local mt = {
    __index = function(t, k)
        local ret = rawget(t, k)
        if ret == nil and type(k) == "userdata" then
            ret = {}
            rawset(t, k, ret)
        end
        return ret
    end
}
setmetatable(_var, mt)
var = _var

local n1 = user.Test()

local m = var[n1]
print(m)
m.value = "hello world"
print(m.value)

var[n1] = nil
print(rawget(var, n1))

n1:print()

--[[
print(n1, n1.value)
n1.value = "hello world"
print(n1, n1.value)

local n2 = user.Test()
print(n2, n2.value)
]]--
