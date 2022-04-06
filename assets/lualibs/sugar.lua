
function defer(self, call, delay)
    self:runAction(action.Steps(
            action.Delay(delay),
            action.Callback(call, self)
    ))
end
