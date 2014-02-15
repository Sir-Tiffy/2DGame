
local buttons = {}

local function MakeButton(sprite, callback)
	buttons[sprite] = callback
end

local function RemoveButton(sprite)
	buttons[sprite] = nil
end


local function onButtonUp(x,y,button)
	if button ~= 1 then return end
	x,y = Screen.ToWorldCoords(x,y)
	for sprite,callback in pairs(buttons) do
		if sprite.Visible and x >= sprite.Position.x and x <= sprite.Position.x + sprite.Size.x and
			y >= sprite.Position.y and y <= sprite.Position.y + sprite.Size.y then
			callback()
		end
	end
end

Input.MouseButtonUp(onButtonUp)

return {new = MakeButton, delete = RemoveButton}