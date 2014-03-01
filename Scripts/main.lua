--setup
local LoadMap = require("scripts/map")

local playerTexture = Texture.new("textures/player.png")
playerTexture:SetFilter("Nearest")
Screen.Title = "Shooty McShmup"
Screen.Width = 800
Screen.Height = 600
Screen.Resizable = false

--constants
local playerSpeed = 8
local playerJumpVelocity = 5
local gravity = -9.8/100

--variables
local playing = false
local player
local objects = {}

local map

local playerMovement = nil

local function OnKeyDown(key)
	if not playing then return end
	if key == 'D' then
		playerMovement = true
	elseif key == 'A' then
		playerMovement = false
	end
end

local function OnKeyUp(key)
	if not playing then return end
	if key == 'D' and playerMovement == true then
		playerMovement = nil
	elseif key == 'A' and playerMovement == false then
		playerMovement = nil
	end
end


local testTarget = Sprite.new()
testTarget.Size = Vector2.new(.25,.25)
testTarget.Layer = 10
local ghost = Sprite.new()
ghost.Size = Vector2.new(1,2)
ghost.Layer = 10

local function ResolveCollisions(startPos,size,endPos)
	testTarget.Position = endPos-testTarget.Size/2
	for _,s in pairs(map.sprites) do
		s.Colour = Vector3.new(1,1,1)
	end
	ghost.Position = endPos

	local collideUp,collideDown,collideLeft,collideRight=false,false,false,false

	local vel = endPos-startPos
	local vx,vy = vel.x,vel.y
	local left = vx<0
	local up = vy>0

	local newX = endPos.x
	local newY = endPos.y

	for x = math.floor(left and startPos.x or startPos.x+size.x), math.floor(left and endPos.x or endPos.x+size.x), left and -1 or 1 do
		for y = math.floor(startPos.y),math.floor(startPos.y+size.y) do
			if map[y] and map[y][x] then
				map.sprites[tostring(Vector2.new(x,y))].Colour = Vector3.new(1)
				if left and x+1 > newX then
					newX = x+1
					collideLeft = true
					break
				elseif not left and x-size.x < newX then
					newX = x-size.x
					collideRight = true
					break
				end
			end
		end
		if collideLeft or collideRight then break end
	end
	print(newX)
	for y = math.floor(up and startPos.y+size.y or startPos.y), math.floor(up and endPos.y+size.y or endPos.y), up and 1 or -1 do
		local row = map[y]
		if row then
			for x = math.floor(newX),math.ceil(newX+size.x-1) do
				local k = tostring(Vector2.new(x,y))
				if map.sprites[k] then map.sprites[k].Colour = Vector3.new(1,1) end
				if row[x] then
					map.sprites[k].Colour = Vector3.new(1)
					if up and y-size.y < newY then
						newY = y-size.y
						collideUp = true
						break
					elseif not up and y+1 > newY then
						newY = y+1
						collideDown = true
						break
					end
				end
			end
			if collideUp or collideDown then break end
		end
	end

	--[[for x = math.floor(endPos.x), math.floor(endPos.x+size.x) do
		for y = math.floor(endPos.y),math.floor(endPos.y+size.y) do
			if map.sprites[tostring(Vector2.new(x,y))] then map.sprites[tostring(Vector2.new(x,y))].Colour = Vector3.new(1,1) end
			if map[y] and map[y][x] then --solid
				map.sprites[tostring(Vector2.new(x,y))].Colour = Vector3.new(1)
				local moveX = 
			end
		end
	end--]]
	--[[
	for x = math.floor(left and startPos.x + size.x or startPos.x), math.floor(left and endPos.x or endPos.x+size.x), left and -1 or 1 do
		for y = math.floor(up and startPos.y or startPos.y + size.y) ,math.floor(up and endPos.y+size.y or endPos.y),up and 1 or -1 do
			if map[y] and map[y][x] then --solid
				map.sprites[tostring(Vector2.new(x,y))].Colour = Vector3.new(1,1)
				if left then
					if x+1 > newX then newX = x+1 end
				else
					if x-size.x < newX then newX = x-size.x end
				end
			end
		end
	end--]]

	ghost.Position = Vector2.new(newX,newY)
	return startPos, collideUp,collideDown,collideLeft,collideRight
end

local function Physics(deltaTime)
	player.velocity = Vector2.new(deltaTime*(playerMovement and playerSpeed or (playerMovement == false and -playerSpeed or 0)),player.velocity.y)
	for i = 1, #objects do
		local obj = objects[i]
		obj.velocity = obj.velocity+Vector2.new(0,gravity*deltaTime)
		local startPos = obj.position
		local endPos = obj.position + obj.velocity*deltaTime
		if (endPos-startPos).length > 1 then
			endPos = startPos + (endPos-startPos).unit
			print("going too fast!")
		end

		--[[local newPos, collideUp, collideDown, collideLeft, collideRight = ResolveCollisions(startPos,object.size,endPos)
		obj.position = newPos
		obj.sprite.Position = newPos
		local vx, vy = obj.velocity.x, obj.velocity.y
		if (collideUp and vy > 0) or (collideDown and vy < 0) then vy = 0 end
		if (collideLeft and vx < 0) or (collideRight and vx > 0) then vx = 0 end
		obj.velocity = Vector2.new(vx,vy)--]]
	end
end

local function NewGame()
	if playing then return error("Game already playing!",2) end
	playing = true


	map = LoadMap("maps/map.map")

	local sprite = player and player.Sprite or Sprite.new()
	sprite.Texture = playerTexture
	sprite.Size = Vector2.new(1,playerTexture.Height/playerTexture.Width)
	sprite.Position = map.playerPos
	sprite.Visible = true
	sprite.UV = Vector4.new(0,0,1,1)
	sprite.Colour = Vector3.new(1,1,1)
	sprite.Layer = 5

	player = {sprite = sprite}
	player.size = sprite.Size
	player.velocity = Vector2.new(0,0)
	player.position = map.playerPos
	player.anchored = false
	player.collision = true
	player.health = 100
	player.position = player.position + Vector2.new(.5,2.5)
	player.sprite.Position = player.position

	objects = {player}


	Camera.Scale = 16
	Camera.Position = player.position + Vector2.new(.5,2)

	do
		local col
		repeat col = Vector3.new(math.random(),math.random(),math.random())
		until col ~= Vector3.new(0,0,0)
		Screen.BackgroundColour = col.unit/4
	end



	playerVelocityX = 0
	playerVelocityY = 0

	Input.KeyDown(OnKeyDown)
	Input.KeyUp(OnKeyUp)
	Input.MouseMove(function(x,y) ResolveCollisions(player.position,player.size,Vector2.new(Screen.ToWorldCoords(x,y))) end)

	while true do
		Physics(1/60)
		Camera.Position = Camera.Position + (player.position+Vector2.new(.5,2)-Camera.Position)*.1
		wait()
	end
end

return NewGame()