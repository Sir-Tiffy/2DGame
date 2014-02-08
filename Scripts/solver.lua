while not ready do wait() end
--[[
CountMines(x,y)
ClickSquare(x,y)
FlagSquare(x,y)]]

local delay = .1

local function TableToString(tab)
	local s = "{"
	for i,v in pairs(tab) do
		s = s.."'"..tostring(i).."' = "
		if false then --if type(v) == "table" then s = s..TableToString(v)
		else s = s..tostring(v) end
		s = s..", "
	end
	s = string.sub(s,1,-3).."}"
	return s
end

local function PrintTable(tab)
	return print(TableToString(tab))
end

local states = {"unclicked","clicked","flagged","open","closed"}
for i = 1, #states do
	states[states[i]] = i
end

local grid,openSquares
local logicRunning = false

local function CreateSet(x,y)
	if not grid[x] or not playing then return end
	local s = grid[x][y]
	if not s or s.num == 0 then return end
	local target = s.num
	local set = {}
	for i = x-1,x+1 do
		local column = grid[i]
		if column then
			for j = y-1,y+1 do
				local s2 = column[j]
				if s2 and (i ~= x or j ~= y) then
					if s2.state == states.flagged then
						target = target -1
					elseif s2.state == states.unclicked then
						table.insert(set,s2)
						table.insert(s2.sets,set)
					end
				end
			end
		end	
	end
	set.target = target
	return set
end

local function GetSquares(x,y)
	if not grid[x] or not playing then return end
	local s = grid[x][y]
	if not s then return end

	assert(s.state ~= states.unclicked, "Tried to get unclicked square")
	assert(s.state ~= states.closed,"Tried to get closed square")
	if s.state ~= states.clicked then return end
	local num =  CountMines(x,y)
	s.num = num

	for _,set in pairs(s.sets) do
		for i = 1,#set do
			if set[i] == s then
				table.remove(set,i)
				break
			end
		end
	end
	s.sets = nil

	if num == 0 then
		s.state = states.closed
		ColourSquare(x,y,Vector3.new(.1,.1,.1))
		for i = x-1, x+1 do
			if grid[i] then
				for j = y-1,y+1 do
					local n = grid[i][j]
					if n and n.state == states.unclicked then
						n.state = states.clicked
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
	if not grid[x] then return end
	local s = grid[x][y]
	if not s or (s.state ~= states.unclicked and s.state ~= states.flagged) then return end
	local didFlag =  FlagSquare(x,y)
	s.state = didFlag and states.flagged or states.unclicked

	if didFlag then
		for _,set in pairs(s.sets) do
			for i = 1, #set,1 do
				if set[i] == s then
					set.target = set.target-1
					table.remove(set,i)
					break
				end
			end
		end
	end
	print(remainingMines,'/',totalMines)
end

local function Click(x,y)
	if not grid[x] or not grid[x][y] or grid[x][y].state ~= states.unclicked or not playing then return end
	if ClickSquare(x,y) == false then return end
	local s = grid[x][y]
	s.state = states.clicked
	return GetSquares(x,y)
end

local function CompareSets(main,other)
	local intersect = {}
	local out = {}
	for i = 1, #main do
		local intersecting = false
		for j = 1, #other do
			if main[i] == other[j] then
				intersecting = true
				break
			end
		end
		table.insert(intersecting and intersect or out, main[i])
	end
	return out, intersect
end

local function GetOtherSets(set)
	local other = {}
	for _,square in ipairs(set) do
		for _,otherSet in pairs(square.sets) do
			if otherSet ~= set then other[otherSet] = true end
		end
	end
	return other
end

local function Logic()
	if logicRunning or not playing or #openSquares == 0 then return end
	logicRunning = true
	local count = 0
	while #openSquares>0 and count <= #openSquares and logicRunning and playing do
		local square = table.remove(openSquares,1)
		local set = square.set
		if not set then
			set = CreateSet(square.x,square.y)
			square.set = set
		end
		local closed = false
		local action = false

		for other in pairs(GetOtherSets(set)) do
			local out,intersecting = CompareSets(set,other)
			local squaresIgnored = #other-#intersecting
			local minMines = other.target - squaresIgnored
			local maxMines = #intersecting
			--minMines = minMines <= 0 and 0 or minMines
			maxMines = maxMines > other.target and other.target or maxMines

			if #out > 0 then
				if set.target - minMines <= 0 then
					for _,v in pairs(out) do
						Click(v.x,v.y)
					end
					action = true
					break
				elseif set.target - maxMines == #out then
					for _,v in pairs(out) do
						Flag(v.x,v.y)
					end
					action = true
					break
				end
			end
		end

		if set.target == #set then
			for i = #set,1,-1 do
				Flag(set[i].x,set[i].y)
			end
			assert(set.target == 0 and #set == 0)
			closed = true
		elseif set.target == 0 then
			while #set > 0 and playing and logicRunning do
				Click(set[1].x,set[1].y)
			end
			closed = true
		end

		if not playing then break end
		if not closed then
			table.insert(openSquares,square)
			count = count + 1
		else
			ColourSquare(square.x,square.y,Vector3.new(.1,.1,.1))
			action = true
		end
		if action then
			count = 0
			if delay > 0 then wait(delay) end
		end
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
			c[y] = {x=x,y=y,state=states.unclicked,sets={}}
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
	if button == 3 and grid[x] and grid[x][y] then
		return PrintTable(grid[x][y])
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
Input.MouseWheel(function(x,y,delta) delay = delay+delta/60 if delay < 0 then delay = 0 end end)
Begin()