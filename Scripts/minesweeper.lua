
local tex = Texture.new("textures/minesweeper.png")
--tex:SetFilter("Nearest")

local UVs = {
	normal = Vector4.new(0,0,20,20),
	down = Vector4.new(24,0,20,20),
	flag = Vector4.new(48,0,20,20),
	mine = Vector4.new(105,0,20,20),
	shadowHorizontal = Vector4.new(72,0,20,5),
	shadowVertical = Vector4.new(96,0,5,20)
}
for i = 1, 4 do
	UVs[i] = Vector4.new((i-1)*22,22,20,20)
	UVs[i+4] = Vector4.new((i-1)*22,43,20,20)
end
do
	local texWidth = tex.width
	local texHeight = tex.height
	for i,v in pairs(UVs) do
		UVs[i] = Vector4.new(v.x/texWidth,1-(v.y+v.w)/texHeight,(v.x+v.z)/texWidth,1-v.y/texHeight)
	end
end

Screen.Resizable = false

local grid = {}
width,height,totalMines,remainingMines=nil
playing = false
local highlightedX, highlightedY
local minesGenerated

local spritePool = {}
function OnNewGame() end
function NewGame(w,h,numMines)
	assert(numMines <= w*h-9)
	width = w
	height = h
	totalMines = numMines
	remainingMines = numMines
	highlightedX = nil
	highlightedY = nil
	minesGenerated = false
	playing = true
	local minePositions = {}

	for _,x in pairs(grid) do
		for _,y in pairs(x) do
			if y.sprite then
				table.insert(spritePool,y.sprite)
				y.sprite.Visible = false
			end
		end
	end
	grid = {}

	for x = 0, width-1 do
		local column = {}
		grid[x] = column
		for y = 0, height-1 do
			local square
			if #spritePool == 0 then square = Sprite.new()
			else
				square = spritePool[#spritePool]
				spritePool[#spritePool] = nil
			end
			square.Colour = Vector3.new(1,1,1)
			square.Size = Vector2.new(1,1)
			square.Texture = tex
			square.Visible = true
			square.UV = UVs.normal
			square.Layer = 2
			square.Position = Vector2.new(x-width/2,height/2-y-1)
			column[y] = {sprite=square, clicked = false, mine = false, flag = false}
		end
	end
	Camera.Position = Vector2.new(0,0)
	Camera.Scale = height
	return OnNewGame()
end

local function GenerateMines(clickX, clickY)
	local mines = {}
	local mineCount = 0
	for x = 0, width-1 do
		for y = 0, height-1 do
			if math.abs(clickX-x) > 1 or math.abs(clickY-y) > 1 then
				mineCount = mineCount + 1
				mines[mineCount] = {x,y}
			end
		end
	end
	for i = 1, totalMines do
		local j = math.random(i,mineCount)
		grid[mines[j][1]][mines[j][2]].mine = true
		mines[j] = mines[i]
	end
	minesGenerated = true
end

local function CountProperty(clickX,clickY,prop)
	if not playing then return end
	if not grid[clickX][clickY].clicked then
		ColourSquare(clickX,clickY,Vector3.new(1,0,1))
		error("Tried to count property "..prop.." of unclicked square "..clickX..", "..clickY,2)
	end
	local count = 0
	for x = clickX-1, clickX+1 do
		if x >= 0 and x < width then
			for y = clickY-1, clickY + 1 do
				if y >= 0 and y < height and grid[x][y][prop] then
					count = count + 1
				end
			end
		end
	end
	return count
end

function CountMines(x,y)
	return CountProperty(x,y,"mine")
end
local function CountSurroundingFlags(x,y)
	return CountProperty(x,y,"flag")
end

local function Lose()
	playing = false
	for x = 0, width-1 do
		for y = 0, height-1 do
			local s = grid[x][y]
			if s.mine and not s.flag then
				s.sprite.UV = UVs.mine
			end
		end
	end
	wait(1)
	NewGame(width,height,totalMines)
	return false
end

function ClickSquare(x,y)
	if x < 0 or x >= width or y < 0 or y >= height or not playing then return end

	local square = grid[x][y]
	if square.flag then return end
	if square.clicked then
		--[[if CountSurroundingFlags(x,y) == CountMines(x,y) then
			for i = x-1, x+1 do
				if i >= 0 and i < width then
					for j = y-1, y+1 do
						if j >= 0 and j < height and (i ~= x or j ~= y) and not grid[i][j].clicked and not grid[i][j].flag then
							ClickSquare(i,j)
						end
					end
				end
			end
		end]]
		return
	end
	square.clicked = true
	--square.sprite.Colour = Vector3.new(1,1,1)
	if not minesGenerated then GenerateMines(x,y) end

	if square.mine then
		return Lose()
	else
		local num = CountMines(x,y)
		if num == 0 then
			square.sprite.UV = UVs.down
			for i = x-1, x+1 do
				for j = y-1, y+1 do
					ClickSquare(i,j)
				end
			end
		else
			square.sprite.UV = UVs[num]
			--square.sprite.Size = Vector2.new(12/20,15/20)
			--square.sprite.Position = square.sprite.Position + Vector2.new((20-12)/20/2,(20-15)/20/2)
		end
	end
end

function ColourSquare(x,y,col)
	if grid[x] and grid[x][y] and playing then grid[x][y].sprite.Colour = col end
end

function FlagSquare(x,y)
	if x < 0 or x >= width or y < 0 or y >= height or not playing then return end
	local square = grid[x][y]
	if square.clicked then return end
	square.flag = not square.flag
	remainingMines = square.flag and remainingMines - 1 or remainingMines + 1
	square.sprite.UV = UVs[square.flag and "flag" or "normal"]
	return square.flag
end

local prevX,prevY

local function ToSquareCoords(x,y)
	local w,h = Screen.Width,Screen.Height
	local realWidth = width/height*h
	local diff = w-realWidth
	return
		math.floor((x-diff/2)*height/h),
		math.floor(y*height/h)
end

local function onMouseMove(x,y)
	x,y = ToSquareCoords(x,y)
	if prevX == x and prevY == y then return end
	if prevX and prevY then
		grid[prevX][prevY].sprite.Colour = Vector3.new(1,1,1)
		prevX = nil
		prevY = nil
	end
	if x < 0 or x >= width or y < 0 or y >= height or not playing then return end
	local s = grid[x][y]
	if s.clicked then return end
	prevX = x
	prevY = y
	--grid[x][y].sprite.Colour = Vector3.new(2,2,2)
end

local function onMouseButtonDown(x,y,button)
	x,y = ToSquareCoords(x,y)
	if button == 2 then return FlagSquare(x,y) end
end

local function onMouseButtonUp(x,y,button)
	x,y = ToSquareCoords(x,y)
	if button == 1 then return ClickSquare(x,y) end
end

NewGame(30,16,99)

--Input.MouseMove(onMouseMove)
--Input.MouseButtonDown(onMouseButtonDown)
--Input.MouseButtonUp(onMouseButtonUp)

ready = true