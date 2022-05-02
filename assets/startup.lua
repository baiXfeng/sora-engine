
import("assets/lualibs/sugar.lua")
import("assets/lualibs/action.lua")
import("assets/lualibs/variable.lua")

scene.push("assets/layouts/title.xml")

--local test = user.Test()
--test:testTable({x=100, y=0.123})

local n = Widget()
n:setPosition({x=100, y=200})
local pos = n:position()
print(pos, pos.x, pos.y, n:parent())

--[[
print(n1, n1.value)
n1.value = "hello world"
print(n1, n1.value)

local n2 = user.Test()
print(n2, n2.value)
]]--
