Screen.Width = 800
Screen.Height = 800
--Screen.Resizable = false

local swapColour = Vector3.new(1,0,0)
local compareColour = Vector3.new(0,1,1)
local defaultColour = Vector3.new(1,1,0)
local sortedColour = Vector3.new(.5,.5,.5)
local pivotColour = Vector3.new(0,0,1)

local array = {}
local arraySize = 0

local waitTime = 0.001

local spritePool = {}

do
	local realWait = wait
	local delay = 0
	wait = function(time)
		time = time or 0
		if time >= 1/60 then return realWait(time) end
		if time == 0 then return end
		delay = delay + time
		if delay >= 1/60 then
			delay = 0
			return realWait(0)
		end
	end

	local realAssert = assert
	assert = function(condition,message,level)
		if not condition then return error(message,(level or 1)+1) end
		return condition
	end
end

local function Clamp(val,min,max)
	return (val<min) and min or ((val>max) and max or val)
end

local function ColourFromHueDeg(hue)
	hue = (hue%360)/60
	return Vector3.new(
		Clamp(math.abs(hue-3)-1,0,1),
		Clamp(2-math.abs(hue-2),0,1),
		Clamp(2-math.abs(hue-4),0,1)
	)
end

local function PositionSprite(i)
	assert(i,"Invalid sprite index!",2)
	assert(i>=1 and i <= arraySize, "Sprite "..i.." out of range!",2)
	local s = array[i]
	val,s = assert(s.i,"Value of index "..i.." missing!",2), assert(s.sprite,"Sprite of index "..i.." missing!",2)
	

	s.Size = Vector2.new(.9/arraySize, val/arraySize)
	s.Position = Vector2.new((i-1+.05)/arraySize,0)
end

local function Swap(a,b)
	array[a].i,array[b].i = array[b].i,array[a].i
	PositionSprite(a)
	return PositionSprite(b)
end

local function Shuffle()
	for i = 1, arraySize-1 do
		local sel = math.random(i,arraySize)
		--array[i].i,array[sel].i = array[sel].i,array[i].i

		Swap(i,sel)

		array[i].sprite.Colour = swapColour
		array[sel].sprite.Colour = swapColour

		--wait(waitTime)
		array[i].sprite.Colour = defaultColour
		array[sel].sprite.Colour = defaultColour
	end
end

local function Bubble()
	for i = arraySize, 1, -1 do
		local swapped = false
		for j = 1, i-1 do
			local a,b = array[j], array[j+1]
			if a.i > b.i then
				Swap(j,j+1)
				--[[a.i,b.i = b.i,a.i
				PositionSprite(j)
				PositionSprite(j+1)]]
				a.sprite.Colour = swapColour
				b.sprite.Colour = swapColour
				swapped = true
			else
				a.sprite.Colour = compareColour
				b.sprite.Colour = compareColour
			end
			wait(waitTime)
			a.sprite.Colour = defaultColour
			b.sprite.Colour = defaultColour
		end
		wait()
		array[i].sprite.Colour = sortedColour
		if not swapped then
			for j = 1, i-1 do
				array[j].sprite.Colour = sortedColour
				assert(array[i].i == i,"Failed to sort!")
			end
			break
		end
	end
end

local function Selection()
	for i = 1, arraySize do
		array[i].sprite.Colour = swapColour
		local lowestI = i
		for j = i+1, arraySize do
			local item = array[j]
			if item.i < array[lowestI].i then
				if lowestI ~= i then array[lowestI].sprite.Colour = defaultColour end
				lowestI = j
				item.sprite.Colour = swapColour
				wait(waitTime)
			else
				item.sprite.Colour = compareColour
				wait(waitTime)
				item.sprite.Colour = defaultColour
			end
		end
		Swap(i,lowestI)
		wait()
		array[lowestI].sprite.Colour = defaultColour
		array[i].sprite.Colour = sortedColour
	end
end

local function Gnome()
	for i = 1, arraySize do
		array[i].sprite.Colour = swapColour
		wait(waitTime)
		local target = i
		for j = i, 2, -1 do
			wait(waitTime)
			local a,b = array[j-1],array[j]
			if a.i > b.i then
				--a.i,b.i = b.i,a.i
				a.sprite.Colour,b.sprite.Colour = b.sprite.Colour,a.sprite.Colour
				--PositionSprite(j-1)
				--PositionSprite(j)
				Swap(j,j-1)
				target = j-1
			else target = j break end
		end
		array[target].sprite.Colour = compareColour
	end
	for i = 1, arraySize do
		array[i].sprite.Colour = sortedColour
		assert(array[i].i == i,"Failed to sort!")
	end
end

local function Insertion()
	for i = 1, arraySize do
		local temp = array[i].i
		array[i].sprite.Colour = compareColour
		for j = i, 2, -1 do
			if array[j-1].i > temp then
				array[j].i = array[j-1].i
				PositionSprite(j)
				array[j].sprite.Colour = swapColour
				wait(waitTime)
				array[j].sprite.Colour = compareColour
			else
				array[j].i = temp
				temp = nil
				PositionSprite(j)
				array[j].sprite.Colour = swapColour
				wait(waitTime)
				array[j].sprite.Colour = compareColour
				break
			end
		end
		if temp then
			array[1].i = temp
			PositionSprite(1)
			array[1].sprite.Colour = swapColour
			wait(waitTime)
			array[1].sprite.Colour = compareColour
		end
	end
	for i = 1, arraySize do
		array[i].sprite.Colour = sortedColour
		assert(array[i].i == i,"Failed to sort!")
	end
end

local function Quick()

	local quicksortMarker = table.remove(spritePool) or Sprite.new()
	quicksortMarker.Visible = true
	quicksortMarker.Size = Vector2.new(1,3/Screen.Height)
	quicksortMarker.Colour = pivotColour
	quicksortMarker.Layer = 2

	local function ChoosePivot(left,right)
		local middle = math.floor((left+right)/2)
		local a,b,c = array[left].i, array[middle].i, array[right].i
		return ((a<b)==(a<c)) and (((a<c)==(b<c)) and middle or right) or left
	end
	local function Partition(left,right,pivotIndex)
		local pivot = array[pivotIndex].i
		Swap(pivotIndex, right)

		array[right].sprite.Colour = pivotColour

		local pivotIndex = left
		for i = left, right-1 do
			if array[i].i <= pivot then
				array[i].sprite.Colour = swapColour
				array[pivotIndex].sprite.Colour = swapColour
				wait(waitTime/2)
				Swap(i,pivotIndex)
				wait(waitTime/2)
				array[pivotIndex].sprite.Colour = defaultColour
				pivotIndex = pivotIndex + 1
				array[i].sprite.Colour = defaultColour
				array[pivotIndex].sprite.Colour = compareColour
			else
				array[i].sprite.Colour = compareColour
				wait(waitTime)
				array[i].sprite.Colour = defaultColour
			end
		end
		Swap(pivotIndex,right)
		array[right].sprite.Colour = defaultColour
		array[pivotIndex].sprite.Colour = sortedColour
		return pivotIndex
	end
	local function DoSort(left,right)
		if left >= right then
			if left == right then
				array[left].sprite.Colour = sortedColour
			end
			return
		end

		local pivotIndex = ChoosePivot(left,right)
		quicksortMarker.Position = Vector2.new(0,array[pivotIndex].i/arraySize - 1/Screen.Height)

		array[pivotIndex].sprite.Colour = pivotColour
		wait(waitTime)
		array[pivotIndex].sprite.Colour = defaultColour

		pivotIndex = Partition(left,right,pivotIndex)

		wait(waitTime)

		DoSort(left,pivotIndex-1)
		return DoSort(pivotIndex+1,right)
	end
	DoSort(1,arraySize)
	quicksortMarker.Visible = false
	quicksortMarker.Layer = 1
	table.insert(spritePool,quicksortMarker)
end

local function Merge()
	local function DoMerge(left,right)
		if left >= right then return end
		local middle = math.floor((left+right)/2)
		DoMerge(left,middle)
		DoMerge(middle+1,right)
		local result = {}
		local j = middle+1

		wait(waitTime)

		array[left].sprite.Colour = compareColour
		array[j].sprite.Colour = compareColour

		for i = left, middle do
			while j <= right and array[j].i < array[i].i do
				array[j].sprite.Colour = compareColour
				if j-1 >= middle+1 then array[j-1].sprite.Colour = swapColour  wait(waitTime) end
				table.insert(result,array[j].i)
				j = j + 1
			end
			array[i].sprite.Colour = compareColour
			table.insert(result,array[i].i)
			if i-1 >= left then array[i-1].sprite.Colour = swapColour end
			wait(waitTime)
		end
		for j = j, right do
			table.insert(result,array[j].i)
			array[j].sprite.Colour = compareColour
			if j-1 >= middle + 1 then array[j-1].sprite.Colour = swapColour  wait(waitTime) end
		end
		array[middle].sprite.Colour = swapColour
		array[right].sprite.Colour = swapColour
		local j = 1
		for i = left, right do
			wait(waitTime)
			array[i].i = result[j]
			array[i].sprite.Colour = defaultColour
			PositionSprite(i)
			j = j+1
		end
	end
	DoMerge(1,arraySize)
	wait(waitTime)
	for i = 1, arraySize do
		array[i].sprite.Colour = sortedColour
		assert(array[i].i == i,"Failed to sort!")
	end
end

local function Shaker()
	for i = 1, arraySize/2 do
		local swapped = false
		for j = i+1,arraySize-i+1 do
			if array[j-1].i > array[j].i then
				Swap(j-1,j)
				swapped = true
				array[j-1].sprite.Colour = swapColour
				array[j].sprite.Colour = swapColour
			else
				array[j-1].sprite.Colour = compareColour
				array[j].sprite.Colour = compareColour
			end
			wait(waitTime)
			array[j-1].sprite.Colour = defaultColour
			array[j].sprite.Colour = defaultColour
		end
		array[arraySize-i+1].sprite.Colour = sortedColour

		if not swapped then
			for i = 1, arraySize do
				array[i].sprite.Colour = sortedColour
			end
			break
		end
		swapped = false

		for j = arraySize-i,i+1,-1 do
			if array[j-1].i > array[j].i then
				Swap(j-1,j)
				swapped = true
				array[j-1].sprite.Colour = swapColour
				array[j].sprite.Colour = swapColour
			else
				array[j-1].sprite.Colour = compareColour
				array[j].sprite.Colour = compareColour
			end
			wait(waitTime)
			array[j-1].sprite.Colour = defaultColour
			array[j].sprite.Colour = defaultColour
		end
		array[i].sprite.Colour = sortedColour

		if not swapped then
			for i = 1, arraySize do
				array[i].sprite.Colour = sortedColour
			end
			break
		end
	end
end

local function Stooge()
	local function DoSort(left,right)
		if left < 1 or right > arraySize or left > right then return end
		if array[left].i > array[right].i then
			Swap(left,right)
			array[left].sprite.Colour = swapColour
			array[right].sprite.Colour = swapColour
		else
			array[left].sprite.Colour = compareColour
			array[right].sprite.Colour = compareColour
		end
		wait(waitTime)
		array[left].sprite.Colour = defaultColour
		array[right].sprite.Colour = defaultColour

		if right-left >= 2 then
			local mid = math.floor((right-left+1)/3)
			DoSort(left,right-mid)
			DoSort(left+mid,right)
			DoSort(left,right-mid)
			for i = left+1, right do
				if array[i].i < array[i-1].i then error() end
			end
		end
	end
	DoSort(1,arraySize)
	for i = 1, arraySize do
		array[i].sprite.Colour = sortedColour
		assert(array[i].i == i,"Failed to sort!")
	end
end

local shellGaps = {1,4,10,23,57,132,301,701}
local function Shell()
	while shellGaps[#shellGaps] < arraySize do
		shellGaps[#shellGaps+1] = math.floor(shellGaps[#shellGaps]*2.25)
	end
	for g = #shellGaps,1,-1 do
		local gap = shellGaps[g]
		if gap < arraySize then
			for i = gap, arraySize do
				array[i].sprite.Colour = compareColour
				for j = i-gap, 1, -gap do
					if array[j].i > array[j+gap].i then
						Swap(j,j+gap)
						array[j].sprite.Colour = swapColour
						array[j+gap].sprite.Colour = swapColour
						wait(waitTime)
						array[j].sprite.Colour = defaultColour
						array[j+gap].sprite.Colour = defaultColour
					else break end
				end
				wait(waitTime)
				array[i].sprite.Colour = defaultColour
			end
		end
	end
	for i = 1, arraySize do
		array[i].sprite.Colour = sortedColour
		assert(array[i].i == i,"Failed to sort!")
	end
end

local function Comb()
	local shrink = 1.3
	local gap = arraySize
	local swapped
	while gap > 1 or swapped do
		if gap > 1 then gap = math.floor(gap/shrink) end
		swapped = false

		for i = 1, arraySize-gap do
			if array[i].i > array[i+gap].i then
				Swap(i,i+gap)
				array[i].sprite.Colour = swapColour
				array[i+gap].sprite.Colour = swapColour
				swapped = true
			else
				array[i].sprite.Colour = compareColour
				array[i+gap].sprite.Colour = compareColour
			end
			wait(waitTime)
			array[i].sprite.Colour = defaultColour
			array[i+gap].sprite.Colour = defaultColour
		end
	end
	for i = 1, arraySize do
		array[i].sprite.Colour = sortedColour
		assert(array[i].i == i,"Failed to sort!")
	end
end

local function Strand()
	local sortIndex = arraySize
	local max = array[sortIndex].i
	for i = sortIndex-1,1,-1 do --max acts as min here
		if array[i].i <= max then
			max = array[i].i
			sortIndex = sortIndex - 1
			Swap(i,sortIndex)
		end
	end

	local subIndex = sortIndex-1
	local arrayTop = arraySize
	while subIndex >= 1 do
		max = array[subIndex].i
		array[subIndex].sprite.Colour = compareColour
		for i = subIndex-1, 1, -1 do
			if array[i].i >= max then
				max = array[i].i
				subIndex = subIndex -1
				Swap(i,subIndex)
				array[i].sprite.Colour = swapColour
				array[subIndex].colour = swapColour
				wait(waitTime)
				array[i].sprite.Colour = defaultColour
				array[subIndex].sprite.Colour = compareColour
			else
				array[i].sprite.Colour = compareColour
				wait(waitTime)
				array[i].sprite.Colour = defaultColour
			end
		end

		wait(waitTime)

		local result = {}
		local j = sortIndex-1
		array[sortIndex].sprite.Colour = compareColour
		array[j].sprite.Colour = compareColour

		for i = sortIndex,arrayTop do
			while j >= subIndex and array[j].i < array[i].i do
				if j+1 <= sortIndex-1 then array[j+1].sprite.Colour = swapColour end
				array[j].sprite.Colour = compareColour
				table.insert(result,array[j].i)
				j = j - 1
				wait(waitTime)
			end
			if i-1 >= sortIndex then array[i-1].sprite.Colour = swapColour end
			array[i].sprite.Colour = compareColour
			table.insert(result,array[i].i)
			wait(waitTime)
		end
		for j = j,subIndex,-1 do
			if j+1 <= sortIndex-1 then array[j+1].sprite.Colour = swapColour end
			array[j].sprite.Colour = compareColour
			table.insert(result,array[j].i)
			wait(waitTime)
		end
		array[arrayTop].sprite.Colour = swapColour
		array[subIndex].sprite.Colour = swapColour


		local count = 1
		for i = subIndex,arrayTop do
			array[i].i = result[count]
			if array[i].i < max then
				arrayTop = i
				array[i].sprite.Colour = defaultColour
			else array[i].sprite.Colour = sortedColour end
			PositionSprite(i)
			count = count + 1
			wait(waitTime)
		end
		sortIndex = subIndex
		subIndex = subIndex-1
		wait(waitTime)
	end
	for i = 1, arraySize do
		array[i].sprite.Colour = sortedColour
		assert(array[i].i == i,"Failed to sort!")
	end
end

local function Heap()
	local function Heapify(i, max)
		local left = 2*i
		local right = left+1
		local largest = i
		if left <= max and array[left].i > array[largest].i then
			largest = left
		end
		if right <= max and array[right].i > array[largest].i then
			largest = right
		end
		if largest ~= i then
			wait(waitTime)
			Swap(i,largest)
			return Heapify(largest,max)
		end
	end
	for i = math.floor(arraySize/2), 1, -1 do
		Heapify(i,arraySize)
	end
	for i = arraySize, 2,-1 do
		Swap(i,1)
		array[i].sprite.Colour = sortedColour
		wait(waitTime)
		Heapify(1,i-1)
	end
	array[1].sprite.Colour = sortedColour
end

local function Begin(num)
	arraySize = num
	for i,v in pairs(array) do
		local s = v.sprite
		table.insert(spritePool,s)
		s.Visible = false
	end
	array = {}
	for i = 1, num do
		local s = table.remove(spritePool) or Sprite.new()
		s.Colour = Vector3.new(1,1,0)
		s.Visible = true
		array[i] = {i=i,sprite = s}
		PositionSprite(i)
	end

	--local sorts = {Heap=Heap,Strand=Strand,Bubble=Bubble, Selection=Selection, Gnome=Gnome, Insertion=Insertion, Quick=Quick, Merge=Merge, Shaker=Shaker, Shell=Shell, Comb=Comb}
	local sorts = {
		"Quick", Quick,
		"Merge", Merge,
		"Heap", Heap,

		"Strand", Strand,
		"Selection", Selection,

		"Shell", Shell,
		"Insertion", Insertion,
		"Gnome", Gnome,


		"Shaker", Shaker,
		"Comb", Comb,
		"Bubble", Bubble

		--[[
radix (msd)
radix (lsd)
intro sort
bitonic sort
		]]
	}
	waitTime = 0.015

	while true do
		for i = 1, #sorts, 2 do
			Shuffle()
			print(sorts[i],"Sort")
			sorts[i+1]()
			wait(1)
		end
	end
end


local function onMouseWheel(x,y,dir)
	if waitTime > 0.1 or (waitTime == 0.1 and dir>=0) then
		waitTime = waitTime + dir/10
	else
		if dir >= 0 then
			waitTime = waitTime*1.1
			if waitTime > 0.1 then waitTime = 0.1 end
		else
			waitTime = waitTime/1.1
		end
		if waitTime < 0.00001 then waitTime = 0.00001 end
	end
end

Input.MouseWheel(onMouseWheel)
return Begin(400)
