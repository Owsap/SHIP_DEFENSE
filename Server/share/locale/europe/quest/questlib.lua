-- Add to the bottom of the document
function wait_time_to_str(timestamp)
	local left_secs = timestamp - get_global_time()
	if left_secs > 0 then
		hours = string.format("%02.f", math.floor(left_secs / 3600))
		mins = string.format("%02.f", math.floor(left_secs / 60 - (hours * 60)))
		secs = string.format("%02.f", math.floor(left_secs - hours * 3600 - mins * 60))
		if tonumber(hours) > 0 then
			return string.format("%d hrs. %d min.", hours, mins)
		else
			return string.format("%d min. %d sec.", mins, secs)
		end
	end
end
