local button = require("scripts/button")

Screen.Width = 800
Screen.Height = 800
Screen.Resizable = false

local s = Sprite.new()
button.new(s,function() print("click") end)

Camera.Position = Vector2.new(0,0)
Camera.Scale = 1



Input.MouseMove(function(x,y)
	local old = Vector2.new(x,y)
	x,y = Screen.ToWorldCoords(x,y)
	--x,y = x/Screen.Width-.5, 1-y/Screen.Height-.5
	s.Position = Vector2.new(x,y)
	print(old,Vector2.new(x,y),s.Position,s.Position+s.Size)
	--s.Colour = (x>=s.Position.x and x<= s.Position.x+s.Size.x and y >= s.Position.y and y <= s.Position.y+s.Size.y) and Vector3.new(1,0,0) or Vector3.new(1,1,1)
end)