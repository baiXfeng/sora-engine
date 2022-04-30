
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
