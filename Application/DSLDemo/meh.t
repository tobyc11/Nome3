mesh = {}

function mesh:each_vertex()
    local v = {}
    v.pos = {type = "intrin", ref = "pos"}
    function v:swap_attr(attr1, attr2)
        local temp = self[attr1]
        self[attr1] = self[attr2]
        self[attr2] = temp
    end
    return v
end

OpMul = {}

function OpMul:New(o)
    o = o or {}
    setmetatable(o, self)
    self.__index = self
    return o
end

v = mesh:each_vertex()
v.new_pos = v.pos
v:swap_attr("pos", "new_pos")
