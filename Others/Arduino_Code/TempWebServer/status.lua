#!/usr/bin/lua

print [[
Content-Type: text/html
]]

localLogHeader = "node_"
localLogPath = "/mnt/web/data/"
globalLogPath = "/mnt/web/data/global.log"
mailList = "/mnt/web/data/mail.list"

function Split(str, delim, maxNb)
	if string.find(str, delim) == nil then
		return { str }
	end
	
	if maxNb == nil or maxNb < 1 then
		maxNb = 0
	end
	
	local result = {}
	local pat = "(.-)" .. delim .. "()"
	local nb = 0
	local lastPos
	
	for part, pos in string.gfind(str, pat) do
		nb = nb + 1
		result[nb] = part
		lastPos = pos
		if nb == maxNb then break end
	end
	
	if nb ~= maxNb then
		result[nb + 1] = string.sub(str, lastPos)
	end
	
	return result
end

function ScanDIR(directory)
	local i, t, popen = 0, {}, io.popen
	for filename in popen('ls -a ' .. directory):lines() do
		i = i + 1
		t[i] = filename
	end
	return t
end

function DrawPage()
	print('<html>\r')
	print('<head>\r')
	print('<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />\r')
	print('<script src="/lib.js"></script>\r')
	print('<title>Environment Status</title>\r')
	print('<script>\r')
	print('window.onload = function(){\r')
	
	for i = 1, table.getn(nodeList) do
		local nodeName = nil
		local tickMax = 0
		local tickMin = 100
		local tickNumber = 0
		local tempSet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
		local humSet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
		
		local nodeFile = io.open(nodeList[i] , "r")
		for line in nodeFile:lines() do
			if line ~= nil then
				local nodeData  = Split(line, "|", nil)
				if nodeData[1] == lTimeDate and tempSet[tonumber(nodeData[2])] == 0 and humSet[tonumber(nodeData[2])] == 0 then
					tempSet[tonumber(nodeData[2])] = nodeData[5]
					humSet[tonumber(nodeData[2])] = nodeData[6]
					nodeName = nodeData[4]
				end
			end
		end
		nodeFile:close()
		
		for j = 1, 24 do
			local loopMax = math.max(tonumber(tempSet[j]), tonumber(humSet[j]))
			local loopMin = math.min(tonumber(tempSet[j]), tonumber(humSet[j]))
			
			if loopMax > tickMax then tickMax = loopMax end
			if loopMin < tickMin then tickMin = loopMin end
		end
		
		tickNumber = math.ceil((tickMax - tickMin) / 10)
		
		print('var container_' .. i .. ' = document.getElementById("node_container_'.. i .. '");\r')
		print('var seriesData_' .. i .. ' = [{name:"Temperature", data:[')
		
		for k = 1, 23 do
			print(tempSet[k] .. ',')
		end
		print(tempSet[24])
		
		print('], color:"RGB(255,0,0)"},{name:"Humidity", data:[')
		
		for l = 1, 23 do
			print(humSet[l] .. ',')
		end
		print(humSet[24])
		
		print('], color:"RGB(0,255,0)"}];\r')
		print('var config_' .. i .. ' = { type: "line", width: 1000, height: 600, series: seriesData_'.. i .. ', container: container_'.. i .. ',')
		print('title: "' .. nodeName .. '", tooltip: { enable: true }, animation: { enable: false }, legend: { enable: true },')
		print('yAxis: { tickSize: ' .. tickNumber .. ', title: "Temperature / Humidity" },')
		print('xAxis: { tickSize: 1, title: "' .. lTimeDate .. '", categories: ["00:00", "01:00", "02:00", "03:00", "04:00", "05:00", "06:00", "07:00", "08:00", "09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00", "16:00", "17:00", "18:00", "19:00", "20:00", "21:00", "22:00", "23:00", "24:00"]}')
		print('};\r')
	end
	
	for i = 1, table.getn(nodeList) do
		print('fishChart.initSettings(config_' .. i .. ');\r')
	end
	
	print('fishChart.render();\r')
	
	print('}\r')
	print('</script>\r')
	print('</head>\r')
	print('<body>\r')
	print('<h2>Environment Status</h2>\r')
	
	print('<div id="global_container" style="width:1000px; height:100px; border:1px; border-style:solid;">\r')
	print('<h3>&nbsp;&nbsp;Time:&nbsp;' .. lTimeShow .. '</h3>')
	print('<h3>&nbsp;&nbsp;Node:&nbsp;' .. table.getn(nodeList) .. '</h3>')
	print('</div></br>')
	
	for i = 1, table.getn(nodeList) do
		print('<div id="node_container_'.. i .. '" style="width:1000px; height:600px; border:1px; border-style:solid;"></div>\r')
	end
	
	print('</body>\r')
	print('</html>\r')
end

function main()
	-- [2014/05/09|08|30|192.168.1.1|30|20]
	lTimeShow = os.date("%Y/%m/%d %H:%M:%S", time)
	lTimeDate = os.date("%Y/%m/%d", time)
	lTimeHour = os.date("%H", time)
	lTimeMinute = os.date("%M", time)
	nodeList = ScanDIR(localLogPath .. localLogHeader .. "*")
	DrawPage()
end

main()
