
_action = {

    Steps = function(...)
        return {
            type = "Sequence",
            actions = {...},
        }
    end,

    Repeat = function(act, repeatCount)
        return {
            type = "Repeat",
            action = act,
            repeatCount = repeatCount,  -- int
        }
    end,

    Delay = function(duration)
        return {
            type = "Delay",
            duration = duration,    -- float
        }
    end,

    Callback = function(lua_func, object)
        return {
            type = "Callback",
            func = lua_func,        -- function
            object = object,        -- table or cpp object
        }
    end,

    ScaleTo = function(scale, duration)
        return {
            type = "ScaleTo",
            scale = scale,          -- {[0], [1]}
            duration = duration,    -- float
        }
    end,

    ScaleBy = function(scale, duration)
        return {
            type = "ScaleBy",
            scale = scale,          -- {[0], [1]}
            duration = duration,    -- float
        }
    end,

    MoveTo = function(position, duration)
        return {
            type = "MoveTo",
            position = position,    -- {[0], [1]}
            duration = duration,    -- float
        }
    end,

    MoveBy = function(position, duration)
        return {
            type = "MoveBy",
            position = position,    -- {[0], [1]}
            duration = duration,    -- float
        }
    end,

    Blink = function(times, duration)
        return {
            type = "Blink",
            times = times,          -- int
            duration = duration,    -- float
        }
    end,
}

action = _action
