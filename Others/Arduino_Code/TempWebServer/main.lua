#!/usr/bin/lua

--HTTP header
print [[
Content-Type: text/plain
]]

warningTemperature = 40

successReturnText = "succ_ardu_openwrt"

localLogPath = "/mnt/web/data/node_"
globalLogPath = "/mnt/web/data/global.log"
mailList = "/mnt/web/data/mail.list"

mailTempPath = "/tmp/warning.mail"

maskCode = "ArduinoEnvNode"

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

function GetPing(target)
	local ret, retText, retNumber, popen = nil, nil, 0, io.popen
	retText = popen('ping -W 1 -c 3 -w 3 ' .. target ..' | grep -m 1 "min/avg/max" | cut -d " " -f 4 | cut -d "/" -f 2'):read()
	if retText ~= nil then
		retNumber = math.ceil(10 * tonumber(retText))
		ret = retNumber
	else
		ret = '0'
	end
	return ret
end

function SendMail()
	local globalLog = io.open(globalLogPath, "a+")
	for line in globalLog:lines() do
		if line ~= nil then
			local globalData  = Split(line, "|", nil)
			if globalData[4] == data[2] and globalData[1] == lTimeDate and globalData[2] == lTimeHour then
				print("Giveup SendMail\n")
				do return end
			end
		end
	end
	globalLog:write(nowText)
	globalLog:close()
	
	local mailFile = io.open(mailTempPath, "w")
	mailFile:write("From: Environment Monitor <admin@brainexplode.com>\n")
	mailFile:write("To: Administrator\n")
	mailFile:write("Subject: Environment Warning\n\n")
	mailFile:write("\n")
	mailFile:write("Time: " .. lTimeMail .. "\n")
	mailFile:write("Node IP: " .. data[2] .. "\n")
	mailFile:write("Node Avg.Ping: " .. lAvgPing .. "\n")
	mailFile:write("Temperature: " .. data[3] .. " C\n")
        mailFile:write("Humidity: " .. data[4] .. " %\n")
        mailFile:write("\n")
	mailFile:close()

	local mailTo = io.open(mailList, "r")
	for line in mailTo:lines() do
		if line ~= nil then
			print("SendMail To: " .. line .. "\n")
			local cmdText = string.format("cat %s | sendmail %s", mailTempPath, line)
			os.execute(cmdText)
		end
	end
	mailTo:close()
end

function main()
	parameter = os.getenv("QUERY_STRING")
	
	if parameter ~= nil then
		data  = Split(parameter, ",", nil)
		
		if table.getn(data) == 4 then
			if data[1] == maskCode then
				print(successReturnText)
				
				-- [2014/05/09|08|30|192.168.1.1|30|20|120]
				lTimeMail = os.date("%Y/%m/%d %H:%M:%S", time)
				lTimeDate = os.date("%Y/%m/%d", time)
				lTimeHour = os.date("%H", time)
				lTimeMinute = os.date("%M", time)
				lAvgPing = GetPing(data[2])
				nowText = lTimeDate .. "|" .. lTimeHour .. "|" .. lTimeMinute .. "|" .. data[2] .. "|" .. data[3] .. "|" .. data[4] .. "|" .. lAvgPing .. "\n"
				
				local logFile = io.open(localLogPath .. data[2], "a+")
				
				local last2Text = nil
				local last1Text = nil
				for line in logFile:lines() do
					if line ~= nil then
						last2Text = last1Text
						last1Text = line
					end
				end
				
				if last1Text == nil or last2Text == nil then
					print("WriteLog\n")
					logFile:write(nowText)
				else
					local last2Data  = Split(last2Text, "|", nil)
					local last1Data  = Split(last1Text, "|", nil)
					
					if last1Data[1] ~= lTimeDate or last1Data[2] ~= lTimeHour or last1Data[3] ~= lTimeMinute then
						print("WriteLog\n")
						logFile:write(nowText)
					end
				end
				logFile:close()
				
				if last2Data[1] == lTimeDate and last1Data[1] == lTimeDate and tonumber(last2Data[5]) >= warningTemperature and tonumber(last1Data[5]) >= warningTemperature and tonumber(data[3]) >= warningTemperature then
					SendMail()
				end
			end
		end
	end
end

main()
