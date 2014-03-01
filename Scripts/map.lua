local function DestroyMap(map)
	for _,s in pairs(map.sprites) do
		s:Destroy()
	end
	map.key = nil
end

local special = {
	PLAYER=0
}

local function LoadMap(filename)
	local file,err = io.open(filename,'r')
	if not file then return error("Failed to open file '"..filename.."': "..err,2) end

	local key = {}
	local sprites = {}
	local map = {key=key,sprites=sprites,destroy = DestroyMap}

	local line = file:read("*l")
	while line do
		local char = string.sub(line,1,1)
		if string.sub(line,2,2) ~= '=' then break end
		local tex = string.sub(line,3)
		if special[tex] then
			key[char] = tex
		else
			local texture = Texture.new(tex)
			texture:SetFilter("Nearest")
			key[char] = {texture=texture,solid = char == string.upper(char)}
		end
		line = file:read("*l")
	end

	if not line then return error("Unexpected end of file!",2) end

	local lines = {line}
	local width = #line
	
	line = file:read()
	while line do
		if #line ~= width then return error("Inconsistent width of map file!",2) end
		table.insert(lines,line)
		line = file:read()
	end
	file:close()
	local height = #lines

	if height == 0 or width == 0 then return error("Map data is empty!",2) end

	local playerPos

	for y = 1,height do
		local row = {}
		map[y-1] = row
		local line = lines[height-y+1]
		for x = 1, width do
			local char = line:sub(x,x)
			if char ~= ' ' then
				if not key[char] then return error("Char '"..char.."' is not in key!",2) end
				local thing = special[key[char]]
				if thing then
					if thing == special.PLAYER then
						if playerPos then return error("Player start position appears more than once in map!",2) end
						playerPos = Vector2.new(x-1,y-1)
					else
						return error("Unknown map key '"..thing.."'!")
					end
				else
					local s = Sprite.new()
					s.Texture = key[char].texture
					s.Position = Vector2.new(x-1,y-1)
					sprites[tostring(s.Position)] = s
					row[x-1] = key[char].solid
				end
			end
		end
	end
	map.playerPos = playerPos
	return map
end

return LoadMap