#!/usr/bin/lua

--HTTP header
print [[
Content-Type: text/plain
]]

successReturnText = "suc_ardu_openwrt"

localLogFile = "/mnt/web/log.txt"
mailTempPath = "/tmp/warning.mail"

maskCode = "abc123"

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

function SendMail(mailTo)
	local mailFile = io.open(mailTempPath, "w")
	
	mailFile:write("From: Environment Monitor <admin@brainexplode.com>\n")
	mailFile:write("To: Administrator\n")
	mailFile:write("Subject: 19F Environment Warning\n\n")
	mailFile:write("\n")
	mailFile:write("Time: " .. ltime .. "\n")
	mailFile:write("Node IP: " .. ta[2] .. "\n")
	mailFile:write("Temperature: " .. ta[3] .. " C\n")
        mailFile:write("Humidity: " .. ta[4] .. " %\n")
        mailFile:write("\n")
	mailFile:close()
	
	local cmdText = string.format("cat %s | sendmail %s", mailTempPath, mailTo)
	os.execute(cmdText)
end

function main()
	parameter = os.getenv("QUERY_STRING")
	
	if parameter ~= nil then
		ta  = Split(parameter, ",", nil)
		
		if table.getn(ta) == 4 then
			if ta[1] == maskCode then
				print(successReturnText)
				
				ltime = os.date()				
				local logFile = io.open(localLogFile, "a")
				
				logFile:write("Time: " .. ltime .. "\n")
				logFile:write("Node IP: " .. ta[2] .. "\n")
				logFile:write("Temperature: " .. ta[3] .. "\n")
				logFile:write("Humidity: " .. ta[4] .. "\n\n")
				
				logFile:close()
				
				if tonumber(ta[3]) >= 40 then
					SendMail("gamekiller123321@gmail.com")
				end
			end
		end
	end
end

main()
