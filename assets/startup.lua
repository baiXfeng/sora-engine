
import("assets/lualibs/sugar.lua")
import("assets/lualibs/action.lua")
import("assets/lualibs/variable.lua")

scene.push("assets/layouts/title.xml")

--[[
print(n1, n1.value)
n1.value = "hello world"
print(n1, n1.value)

local n2 = user.Test()
print(n2, n2.value)
]]--
