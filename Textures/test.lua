
local file = io.open("test.bmp","wb")
local count = 0
local function write(num, len)
	count = count + len
	for i = 0, len-1 do
		file:write(string.char(bit32.extract(num,i*8,8)))
	end
end

--file header
file:write("BM") --header field
write(196660,4) --filesize
write(0,4) --reserved
write(54,4) --offset
--dib header
write(40,4) --header size
write(256,4) --width
write(256,4) --height
write(1,2) -- colour planes
write(24,2) -- bits per pixel
write(0,4) --compression method
write(0,4) --image size
write(256,4) --horizontal resolution
write(256,4) --vertical resolution
write(0,4) --colours in palette
write(0,4) --important colours
print(count)
for y = 255, 0, -1 do
	for x = 0, 255 do
		write(x,1)
		write(0,1)
		write(y,1)
	end
end
file:close()
print(count)