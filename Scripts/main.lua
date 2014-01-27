

local function GetColourFromHue(hue)
	local modx = (hue*3/math.pi)%(2*math.pi)
	return Vector3.new(math.abs(modx-3)-1,2-math.abs(modx-2),2-math.abs(modx-4))
end
local dir = 0
local function OnKeyDown(key)
	if key == 'A' then dir = -1
	elseif key == 'D' then dir = 1
	end
end

local function OnKeyUp(key)
	if (dir == -1 and key == 'A') or (dir == 1 and key == 'D') then dir = 0 end
end

Camera.Scale = math.sqrt(18)
Camera.Position = Vector2.new(1.5,1.5)
Camera.Rotation = math.pi/4

local p = {
	Vector2.new(0,0),
	Vector2.new(2,0),
	Vector2.new(1,1),
	Vector2.new(0,2),
	Vector2.new(2,2),
}
local t = {
	"textures/test.bmp",
	"textures/larry.bmp",
	"textures/icon.bmp",
	"textures/fence.bmp",
	"textures/meseara yi.bmp",
}

for i,v in pairs(p) do
	local s = Sprite.new()
	s.Position = v
	s.Size = Vector2.new(1,1)
	s.Colour = Vector3.new(1,1,1)
	s.Texture = Texture.new(t[i])
	s.UV = Vector4.new(0,0,1,1)
end
t = nil
p = nil

local time = 0

Input.KeyDown(OnKeyDown)
Input.KeyUp(OnKeyUp)

while true do
	local _, delta = wait()
	if dir then
		time = time - delta*dir
		Camera.Rotation = time
		Camera.Scale = 3+math.sqrt(2)*math.abs(math.sin(time*2))
	end
end