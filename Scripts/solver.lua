while not ready do wait() end
--[[
CountMines(x,y)
ClickSquare(x,y)
FlagSquare(x,y)]]

local delay = .1


local states = {"unclicked","clicked","flagged","open","closed"}
for i = 1, #states do
	states[states[i]] = i
end

local grid,openSquares
local logicRunning = false

local function GetSquares(x,y)
	if not grid[x] then return end
	local s = grid[x][y]

	if not s then return end
	assert(s.state ~= states.unclicked, "Tried to get unclicked square")
	assert(s.state ~= states.closed,"Tried to get closed square")
	if s.state ~= states.clicked then return end
	if s.num == 0 then
		s.state = states.closed
		ColourSquare(x,y,Vector3.new(.1,.1,.1))
		for i = x-1, x+1 do
			if grid[i] then
				for j = y-1,y+1 do
					local n = grid[i][j]
					if n and n.state == states.unclicked then
						n.state = states.clicked
						n.num = CountMines(i,j)
						GetSquares(i,j)
					end
				end
			end
		end
	else
		s.state = states.open
		return table.insert(openSquares,1,s)
	end
end

local function GetSurroundingSquares(x,y)
	local unclicked = {}
	local flagged = {}
	for i = x-1, x+1 do
		if grid[i] then
			for j = y-1,y+1 do
				local s = grid[i][j]
				if s and (i ~= x or j ~= y) then
					if s.state == states.unclicked then
						table.insert(unclicked,s)
					elseif s.state == states["flagged"] then
						table.insert(flagged,s)
					end
				end
			end
		end
	end
	return unclicked,flagged
end

local function Flag(x,y)
	if not grid[x] or not grid[x][y] or (grid[x][y].state ~= states.unclicked and grid[x][y].state ~= states.flagged) then return end
	if FlagSquare(x,y) then
		grid[x][y].state = states.flagged
	else
		grid[x][y].state = states.unclicked
	end
	print(remainingMines)
end

local function Click(x,y)
	if not grid[x] or not grid[x][y] or grid[x][y].state ~= states.unclicked then return end
	if ClickSquare(x,y) == false then return end
	local s = grid[x][y]
	s.state = states.clicked
	s.num = CountMines(x,y)
	return GetSquares(x,y)
end

local function Logic()
	if logicRunning or not playing or #openSquares == 0 then return end
	logicRunning = true
	local count = 0
	while #openSquares>0 and count <= #openSquares and logicRunning and playing do
		local s = table.remove(openSquares,1)
		local unclicked, flagged = GetSurroundingSquares(s.x,s.y)
		for i = 1, #unclicked do ColourSquare(unclicked[i].x,unclicked[i].y,Vector3.new(1,0,0)) end
		for i = 1, #flagged do ColourSquare(flagged[i].x,flagged[i].y,Vector3.new(0,0,1)) end
		ColourSquare(s.x,s.y,Vector3.new(1,1,0))
		local closed = false
		assert(s.num ~= 0, "Added empty square to open set")
		if #unclicked == 0 then
			assert(#flagged == s.num)
			closed = true
		elseif #unclicked == s.num - #flagged then
			for i = 1, #unclicked do
				Flag(unclicked[i].x,unclicked[i].y)
			end
			closed = true
		elseif #flagged == s.num then
			for i = 1, #unclicked do
				Click(unclicked[i].x,unclicked[i].y)
			end
			closed = true
		end
		if closed then
			s.state = states.closed
			count = 0
		elseif playing then
			table.insert(openSquares,s)
			count = count + 1
		end
		if closed then
			if playing and delay > 0 then wait(delay) end
			ColourSquare(s.x,s.y,Vector3.new(.1,.1,.1))
		else
			ColourSquare(s.x,s.y,Vector3.new(1,1,1))

		end
		for i = 1, #unclicked do if unclicked[i].state ~= states.closed then ColourSquare(unclicked[i].x,unclicked[i].y,Vector3.new(1,1,1)) end end
		for i = 1, #flagged do if flagged[i].state ~= states.closed then ColourSquare(flagged[i].x,flagged[i].y,Vector3.new(1,1,1)) end end
	end
	logicRunning = false
end

local function Begin()
	grid = {}
	openSquares = {}
	logicRunning = false
	for x = 0, width-1 do
		local c = {}
		grid[x] = c
		for y = 0, height-1 do
			c[y] = {x=x,y=y,state=states.unclicked}
		end
	end
	Click(math.floor(width/2),math.floor(height/2))
	return Logic()
end
OnNewGame = Begin

local function ToSquareCoords(x,y)
	local w,h = Screen.Width,Screen.Height
	local realWidth = width/height*h
	local diff = w-realWidth
	return
		math.floor((x-diff/2)*height/h),
		math.floor(y*height/h)
end
local function onMouseButtonDown(x,y,button)
	x,y = ToSquareCoords(x,y)
	if button == 2 then
		Flag(x,y)
		return Logic()
	end
end
local function onMouseButtonUp(x,y,button)
	x,y = ToSquareCoords(x,y)
	if button == 1 then
		Click(x,y)
		return Logic()
	end
end

Input.MouseButtonDown(onMouseButtonDown)
Input.MouseButtonUp(onMouseButtonUp)
Input.MouseWheel(function(x,y,delta) delay = delay+delta/60 if delay < 0 then delay = 0 end print(delta) end)
Begin()